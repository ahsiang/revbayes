#include "ArgumentRule.h"
#include "ArgumentRules.h"
#include "ContinuousStochasticNode.h"
#include "OrnsteinUhlenbeckProcess.h"
#include "Probability.h"
#include "Process_OrnsteinUhlenbeck.h"
#include "Real.h"
#include "RealPos.h"

using namespace RevLanguage;


/**
 * Default constructor.
 *
 * The default constructor does nothing except allocating the object.
 */
OrnsteinUhlenbeckProcess::OrnsteinUhlenbeckProcess() : ContinuousDistribution()
{
    
}


/**
 * Create a new internal distribution object.
 *
 * This function simply dynamically allocates a new internal distribution object that can be
 * associated with the variable. The internal distribution object is created by calling its
 * constructor and passing the distribution-parameters (other DAG nodes) as arguments of the
 * constructor. The distribution constructor takes care of the proper hook-ups.
 *
 * \return A new internal distribution object.
 */
RevBayesCore::OrnsteinUhlenbeckProcess* OrnsteinUhlenbeckProcess::createDistribution( void ) const
{
    
    // get the parameters
    RevBayesCore::TypedDagNode<double>* x           = static_cast<const Real &>( x0->getRevObject() ).getDagNode();
    RevBayesCore::TypedDagNode<double>* th          = static_cast<const Real &>( theta->getRevObject() ).getDagNode();
    RevBayesCore::TypedDagNode<double>* s           = static_cast<const RealPos &>( sigma->getRevObject() ).getDagNode();
    RevBayesCore::TypedDagNode<double>* a           = static_cast<const RealPos &>( alpha->getRevObject() ).getDagNode();
    RevBayesCore::TypedDagNode<double>* ti          = static_cast<const RealPos &>( time->getRevObject() ).getDagNode();
    RevBayesCore::OrnsteinUhlenbeckProcess*   d     = new RevBayesCore::OrnsteinUhlenbeckProcess(x, th, a, s, ti);
    
    return d;
}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
OrnsteinUhlenbeckProcess* OrnsteinUhlenbeckProcess::clone( void ) const
{
    
    return new OrnsteinUhlenbeckProcess(*this);
}


/**
 * Get Rev type of object
 *
 * \return The class' name.
 */
const std::string& OrnsteinUhlenbeckProcess::getClassType(void)
{
    
    static std::string revType = "OrnsteinUhlenbeckProcess";
    
    return revType;
}


/**
 * Get class type spec describing type of an object from this class (static).
 *
 * \return TypeSpec of this class.
 */
const TypeSpec& OrnsteinUhlenbeckProcess::getClassTypeSpec(void)
{
    
    static TypeSpec revTypeSpec = TypeSpec( getClassType(), new TypeSpec( ContinuousDistribution::getClassTypeSpec() ) );
    
    return revTypeSpec;
}


/**
 * Get the alternative Rev names (aliases) for the constructor function.
 *
 * \return Rev aliases of constructor function.
 */
std::vector<std::string> OrnsteinUhlenbeckProcess::getDistributionFunctionAliases( void ) const
{
    // create alternative constructor function names variable that is the same for all instance of this class
    std::vector<std::string> a_names;
    a_names.push_back( "OU" );
    
    return a_names;
}


/**
 * Get the Rev name for the distribution.
 * This name is used for the constructor and the distribution functions,
 * such as the density and random value function
 *
 * \return Rev name of constructor function.
 */
std::string OrnsteinUhlenbeckProcess::getDistributionFunctionName( void ) const
{
    // create a distribution name variable that is the same for all instance of this class
    std::string d_name = "OrnsteinUhlenbeck";
    
    return d_name;
}


/**
 * Get the author(s) of this function so they can receive credit (and blame) for it.
 */
std::vector<std::string> OrnsteinUhlenbeckProcess::getHelpAuthor(void) const
{
    // create a vector of authors for this function
    std::vector<std::string> authors;
    authors.push_back( "Sebastian Hoehna" );
    
    return authors;
}


/**
 * Get the (brief) description for this function
 */
std::vector<std::string> OrnsteinUhlenbeckProcess::getHelpDescription(void) const
{
    // create a variable for the description of the function
    std::vector<std::string> descriptions;
    descriptions.push_back( "A Bernoulli-distributed random variable takes the value 1 with probability p and the value 0 with probability 1-p." );
    
    return descriptions;
}


/**
 * Get the more detailed description of the function
 */
std::vector<std::string> OrnsteinUhlenbeckProcess::getHelpDetails(void) const
{
    // create a variable for the description of the function
    std::vector<std::string> details;
    
    return details;
}


/**
 * Get an executable and instructive example.
 * These example should help the users to show how this function works but
 * are also used to test if this function still works.
 */
std::string OrnsteinUhlenbeckProcess::getHelpExample(void) const
{
    // create an example as a single string variable.
    std::string example = "";
    
    example += "p ~ dnBeta(1.0,1.0)\n";
    example += "x ~ dnBernoulli(p)\n";
    example += "x.clamp(1)\n";
    example += "moves[1] = mvSlide(p, delta=0.1, weight=1.0)\n";
    example += "monitors[1] = screenmonitor(printgen=1000, separator = "	", speciation)\n";
    example += "mymodel = model(p)\n";
    example += "mymcmc = mcmc(mymodel, monitors, moves)\n";
    example += "mymcmc.burnin(generations=20000,tuningInterval=100)\n";
    example += "mymcmc.run(generations=200000)\n";
    
    return example;
}


/**
 * Get some references/citations for this function
 *
 */
std::vector<RevBayesCore::RbHelpReference> OrnsteinUhlenbeckProcess::getHelpReferences(void) const
{
    // create an entry for each reference
    std::vector<RevBayesCore::RbHelpReference> references;
    
    
    return references;
}


/**
 * Get the names of similar and suggested other functions
 */
std::vector<std::string> OrnsteinUhlenbeckProcess::getHelpSeeAlso(void) const
{
    // create an entry for each suggested function
    std::vector<std::string> see_also;
    see_also.push_back( "dnBinomial" );
    
    
    return see_also;
}


/**
 * Get the title of this help entry
 */
std::string OrnsteinUhlenbeckProcess::getHelpTitle(void) const
{
    // create a title variable
    std::string title = "Bernoulli Distribution";
    
    return title;
}


/**
 * Get the member rules used to create the constructor of this object.
 *
 * The member rules of the branch rate jump process are:
 * (1) the min of the uniform distribution.
 * (2) the max of the uniform distribution.
 * (3) the standard deviation of the normal.
 * (4) the probability of belonging to distribution one.
 *
 * \return The member rules.
 */
const MemberRules& OrnsteinUhlenbeckProcess::getParameterRules(void) const
{
    
    static MemberRules distMemberRules;
    static bool rulesSet = false;
    
    if ( !rulesSet )
    {
        distMemberRules.push_back( new ArgumentRule( "x0"   , Real::getClassTypeSpec()   , "The root parameter value.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY  ) );
        distMemberRules.push_back( new ArgumentRule( "theta", Real::getClassTypeSpec()   , "The location of the optimum parameter.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY  ) );
        distMemberRules.push_back( new ArgumentRule( "alpha", RealPos::getClassTypeSpec(), "The attraction to the optimum parameter.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY  ) );
        distMemberRules.push_back( new ArgumentRule( "sigma", RealPos::getClassTypeSpec(), "The scaling parameter of the time.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
        distMemberRules.push_back( new ArgumentRule( "time" , RealPos::getClassTypeSpec(), "The duration of the process.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
        
        rulesSet = true;
    }
    
    return distMemberRules;
}


/**
 * Get type-specification on this object (non-static).
 *
 * \return The type spec of this object.
 */
const TypeSpec& OrnsteinUhlenbeckProcess::getTypeSpec( void ) const
{
    
    static TypeSpec ts = getClassTypeSpec();
    
    return ts;
}


/** Print value for user */
void OrnsteinUhlenbeckProcess::printValue(std::ostream& o) const
{
    
    o << "OU(x0=";
    if ( x0 != NULL ) {
        o << x0->getName();
    }
    else
    {
        o << "?";
    }
    o << ", theta=";
    if ( theta != NULL )
    {
        o << theta->getName();
    }
    else
    {
        o << "?";
    }
    o << ", alpha=";
    if ( alpha != NULL )
    {
        o << alpha->getName();
    }
    else
    {
        o << "?";
    }
    o << ", sigma=";
    if ( sigma != NULL )
    {
        o << sigma->getName();
    }
    else
    {
        o << "?";
    }
    o << ", time=";
    if ( time != NULL )
    {
        o << time->getName();
    }
    else
    {
        o << "?";
    }
    o << ")";
}


/**
 * Set a member variable.
 *
 * Sets a member variable with the given name and store the pointer to the variable.
 * The value of the variable might still change but this function needs to be called again if the pointer to
 * the variable changes. The current values will be used to create the distribution object.
 *
 * \param[in]    name     Name of the member variable.
 * \param[in]    var      Pointer to the variable.
 */
void OrnsteinUhlenbeckProcess::setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var)
{
    
    if ( name == "x0" )
    {
        x0 = var;
    }
    else if ( name == "theta" )
    {
        theta = var;
    }
    else if ( name == "alpha" )
    {
        alpha = var;
    }
    else if ( name == "sigma" )
    {
        sigma = var;
    }
    else if ( name == "time" )
    {
        time = var;
    }
    else
    {
        Distribution::setConstParameter(name, var);
    }
}
