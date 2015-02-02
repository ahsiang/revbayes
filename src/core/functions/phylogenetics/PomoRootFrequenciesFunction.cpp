

#include "PomoRootFrequenciesFunction.h"
//#include "GtrRateMatrix.h"
#include "RbException.h"

using namespace RevBayesCore;

PomoRootFrequenciesFunction::PomoRootFrequenciesFunction(const TypedDagNode<std::vector<double> > *fnrf, const TypedDagNode< double > *fopar, const TypedDagNode<std::vector<double> > *mr, const TypedDagNode< int > *ps) : TypedFunction< std::vector<double> > ( new std::vector<double> () ), fixedNucleotideRootFrequencies( fnrf ), frequencyOfPolymorphismsAtTheRoot( fopar ), mutationRates(mr), populationSize(ps)  {
    useMutationMatrix = false;
    for (size_t i = 0; i < 4+6* (populationSize->getValue()-1); ++i) {
        static_cast< std::vector<double>* >(value)->push_back(0.0);
    }
    // add the lambda parameter as a parent
    addParameter( fixedNucleotideRootFrequencies );
    addParameter( frequencyOfPolymorphismsAtTheRoot );
    addParameter( mutationRates );
    addParameter( populationSize );

    update();
}


PomoRootFrequenciesFunction::PomoRootFrequenciesFunction(const TypedDagNode<std::vector<double> > *fnrf, const TypedDagNode< double > *fopar, const TypedDagNode< RateMatrix > *mm, const TypedDagNode< int > *ps): TypedFunction< std::vector<double> > ( new std::vector<double> () ), fixedNucleotideRootFrequencies( fnrf ), frequencyOfPolymorphismsAtTheRoot( fopar ), mutationMatrix( mm ), populationSize(ps)  {
    useMutationMatrix = true;
    for (size_t i = 0; i < 4+6* (populationSize->getValue()-1); ++i) {
        static_cast< std::vector<double>* >(value)->push_back(0.0);
    }
    // add the lambda parameter as a parent
    addParameter( fixedNucleotideRootFrequencies );
    addParameter( frequencyOfPolymorphismsAtTheRoot );
    addParameter( mutationMatrix );
    addParameter( populationSize );
    
    update();
}


PomoRootFrequenciesFunction::~PomoRootFrequenciesFunction( void ) {
    // We don't delete the parameters, because they might be used somewhere else too. The model needs to do that!
}



PomoRootFrequenciesFunction* PomoRootFrequenciesFunction::clone( void ) const
{
    return new PomoRootFrequenciesFunction( *this );
}


void PomoRootFrequenciesFunction::update( void )
{
    std::vector<double> mr ;
    if (useMutationMatrix) {
        mr = setMutationRates( mutationMatrix->getValue() );
    }
    else {
        mr = mutationRates->getValue();
    }
    
    // get the information from the arguments for reading the file
    const std::vector<double>& fnrf = fixedNucleotideRootFrequencies->getValue();
    const double& fopar = frequencyOfPolymorphismsAtTheRoot->getValue();
    
    const unsigned int& ps = populationSize->getValue();
    
    double OneMinusFopar = 1 - fopar;
    // compute all root frequencies as in De Maio, Schlötterer, Kosiol 2013
    //First, the monoallelic states
    static_cast< std::vector<double>* >(value)->at(0) = fnrf[0] * OneMinusFopar;
    static_cast< std::vector<double>* >(value)->at(1) = fnrf[1] * OneMinusFopar;
    static_cast< std::vector<double>* >(value)->at(2) = fnrf[2] * OneMinusFopar;
    static_cast< std::vector<double>* >(value)->at(3) = fnrf[3] * OneMinusFopar;
    //Then, the biallelic states
    int currentCell = 4;
    for (unsigned int i = 0; i < 3; ++i) {
        for (unsigned int j = i+1; j < 4; ++j) {
            //For each frequency
            for (unsigned int f = 1; f < ps; ++f) {
                static_cast< std::vector<double>* >(value)->at(currentCell) = fopar * ( ( fnrf[j]*mr[j*3+i]*1/(double)f) + ( fnrf[i]*mr[i*3+j-1] * 1/(double)(ps-f) ) );
                currentCell ++;
            }
        }
    }
    //Normalization, in case the polymorphic states do not add up to fopar
    double sum = 0.0;
    for (unsigned int i = 4; i < static_cast< std::vector<double>* >(value)->size(); ++i) {
        sum += static_cast< std::vector<double>* >(value)->at(i);
    }
    double normalizer = fopar/sum;
    for (unsigned int i = 4; i < static_cast< std::vector<double>* >(value)->size(); ++i) {
        static_cast< std::vector<double>* >(value)->at(i) *= normalizer;
    }
    

}




void PomoRootFrequenciesFunction::swapParameterInternal(const DagNode *oldP, const DagNode *newP)
{
    
    if (oldP == fixedNucleotideRootFrequencies)
    {
        fixedNucleotideRootFrequencies = static_cast<const TypedDagNode<std::vector<double> >* >( newP );
    }
    else if (oldP == frequencyOfPolymorphismsAtTheRoot)
    {
        frequencyOfPolymorphismsAtTheRoot = static_cast<const TypedDagNode< double >* >( newP );
    }
    else if (oldP == mutationRates)
    {
        mutationRates = static_cast<const TypedDagNode<std::vector<double> >* >( newP );
    }
    else if (oldP == populationSize)
    {
        populationSize = static_cast<const TypedDagNode< int >* >( newP );
    }

    
}



std::vector<double> PomoRootFrequenciesFunction::setMutationRates(const RateMatrix& mm) {
    std::vector<double> r;
    r.push_back( mm[0][1] );
    r.push_back( mm[0][2] );
    r.push_back( mm[0][3] );
    r.push_back( mm[1][0] );
    r.push_back( mm[1][2] );
    r.push_back( mm[1][3] );
    r.push_back( mm[2][0] );
    r.push_back( mm[2][1] );
    r.push_back( mm[2][3] );
    r.push_back( mm[3][0] );
    r.push_back( mm[3][1] );
    r.push_back( mm[3][2] );
    return r;
}

