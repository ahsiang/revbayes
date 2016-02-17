#include "ArgumentRule.h"
#include "ArgumentRules.h"
#include "ConstantNode.h"
#include "MaxIterationStoppingRule.h"
#include "MaximumLikelihoodAnalysis.h"
#include "Model.h"
#include "Natural.h"
#include "OptionRule.h"
#include "RbException.h"
#include "RealPos.h"
#include "RlMaximumLikelihoodAnalysis.h"
#include "RlModel.h"
#include "RlMonitor.h"
#include "RlMove.h"
#include "RlStoppingRule.h"
#include "RlString.h"
#include "StoppingRule.h"
#include "TypeSpec.h"
#include "WorkspaceVector.h"


using namespace RevLanguage;

MaximumLikelihoodAnalysis::MaximumLikelihoodAnalysis(void) : WorkspaceToCoreWrapperObject<RevBayesCore::MaximumLikelihoodAnalysis>( )
{
    
    initializeMethods();
    
}

MaximumLikelihoodAnalysis::MaximumLikelihoodAnalysis(RevBayesCore::MaximumLikelihoodAnalysis *m) : WorkspaceToCoreWrapperObject<RevBayesCore::MaximumLikelihoodAnalysis>( m )
{
    
    initializeMethods();
    
}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
MaximumLikelihoodAnalysis* MaximumLikelihoodAnalysis::clone(void) const
{
    
    return new MaximumLikelihoodAnalysis(*this);
}


/* Map calls to member methods */
RevPtr<RevVariable> MaximumLikelihoodAnalysis::executeMethod(std::string const &name, const std::vector<Argument> &args, bool &found)
{
    
    if (name == "run")
    {
        found = true;
        
        // get the member with give index
        double e = static_cast<const RealPos &>( args[0].getVariable()->getRevObject() ).getValue();
        
        value->run( e );
        
        return NULL;
    }
    else if (name == "variable")
    {
        found = true;
        
        // get the member with give index
        const std::string &n = static_cast<const RlString &>( args[0].getVariable()->getRevObject() ).getValue();
        
        RevBayesCore::Model &m = value->getModel();
        
        // get the DAG nodes of the model
        std::vector<RevBayesCore::DagNode *> current_ordered_nodes = m.getOrderedStochasticNodes();
        
        for (size_t j = 0; j < current_ordered_nodes.size(); ++j)
        {
            RevBayesCore::DagNode *the_node = current_ordered_nodes[j];
            
            if ( the_node->getName() == n )
            {
                RevPtr<RevVariable>& tmp = Workspace::userWorkspace().getVariable( n );
                RevObject *c = tmp->getRevObject().clone();
                dynamic_cast<AbstractModelObject*>( c )->setDagNode( the_node );
                c->makeConstantValue();
                return new RevVariable( c );
            }
            
        }
        
        return NULL;
    }
    
    return RevObject::executeMethod( name, args, found );
}


/** Get Rev type of object */
const std::string& MaximumLikelihoodAnalysis::getClassType(void)
{
    
    static std::string revType = "MaximumLikelihoodAnalysis";
    
    return revType;
}

/** Get class type spec describing type of object */
const TypeSpec& MaximumLikelihoodAnalysis::getClassTypeSpec(void)
{
    
    static TypeSpec revTypeSpec = TypeSpec( getClassType(), new TypeSpec( WorkspaceToCoreWrapperObject<RevBayesCore::MaximumLikelihoodAnalysis>::getClassTypeSpec() ) );
    
    return revTypeSpec;
}



/** Return member rules (no members) */
const MemberRules& MaximumLikelihoodAnalysis::getParameterRules(void) const
{
    
    static MemberRules memberRules;
    static bool rulesSet = false;
    
    if ( !rulesSet )
    {
        
        memberRules.push_back( new ArgumentRule("model"   , Model::getClassTypeSpec()                   , "The model graph.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
        memberRules.push_back( new ArgumentRule("monitors", WorkspaceVector<Monitor>::getClassTypeSpec(), "The monitors used for this analysis.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
        memberRules.push_back( new ArgumentRule("moves"   , WorkspaceVector<Move>::getClassTypeSpec()   , "The moves used for this analysis.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
        
        std::vector<std::string> options;
        options.push_back( "sequential" );
        options.push_back( "random" );
        options.push_back( "single" );
        
        memberRules.push_back( new OptionRule( "moveschedule", new RlString( "random" ), options, "The strategy how the moves are used." ) );
        
        rulesSet = true;
    }
    
    return memberRules;
}

/** Get type spec */
const TypeSpec& MaximumLikelihoodAnalysis::getTypeSpec( void ) const
{
    
    static TypeSpec typeSpec = getClassTypeSpec();
    
    return typeSpec;
}


void MaximumLikelihoodAnalysis::initializeMethods()
{
    
    ArgumentRules* runArgRules = new ArgumentRules();
    runArgRules->push_back( new ArgumentRule( "epsilon", RealPos::getClassTypeSpec(), "The minimum improvement in the last interval.", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RealPos(0.001) ) );
    methods.addFunction( new MemberProcedure( "run", RlUtils::Void, runArgRules) );

    ArgumentRules* varArgRules = new ArgumentRules();
    varArgRules->push_back( new ArgumentRule( "name", RlString::getClassTypeSpec(), "The name of the variable.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
    methods.addFunction( new MemberProcedure( "variable", AbstractModelObject::getClassTypeSpec(), varArgRules) );

    
}


/**
 * Print value
 */
void MaximumLikelihoodAnalysis::printValue(std::ostream &o) const
{
    
    o << "MaximumLikelihoodAnalysis";
}


/**
 * Set a member variable
 */
void MaximumLikelihoodAnalysis::setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var)
{
    
    if ( name == "model")
    {
        model = var;
    }
    else if ( name == "moves")
    {
        moves = var;
    }
    else if ( name == "monitors")
    {
        monitors = var;
    }
    else if ( name == "moveschedule")
    {
        moveschedule = var;
    }
    else
    {
        WorkspaceToCoreWrapperObject<RevBayesCore::MaximumLikelihoodAnalysis>::setConstParameter(name, var);
    }
    
}
