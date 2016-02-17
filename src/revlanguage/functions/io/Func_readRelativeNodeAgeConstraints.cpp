#include "AbstractHomologousDiscreteCharacterData.h"
#include "ArgumentRule.h"
#include "ConstantNode.h"
#include "RelativeNodeAgeConstraints.h"
#include "RelativeNodeAgeConstraintsReader.h"
#include "Ellipsis.h"
#include "Func_readRelativeNodeAgeConstraints.h"
#include "ModelVector.h"
#include "RbException.h"
#include "RbFileManager.h"
#include "RevNullObject.h"
#include "RlBoolean.h"
#include "RlRelativeNodeAgeConstraints.h"
#include "RlHomologousDiscreteCharacterData.h"
#include "RlString.h"
#include "RlUtils.h"
#include "StringUtilities.h"
#include "RlUserInterface.h"

#include <map>
#include <set>
#include <sstream>


using namespace RevLanguage;

/** Clone object */
Func_readRelativeNodeAgeConstraints* Func_readRelativeNodeAgeConstraints::clone( void ) const
{
    
    return new Func_readRelativeNodeAgeConstraints( *this );
}


/** Execute function */
RevPtr<RevVariable> Func_readRelativeNodeAgeConstraints::execute( void )
{
    
    // get the information from the arguments for reading the file
    const RlString& fn = static_cast<const RlString&>( args[0].getVariable()->getRevObject() );
    
    RevBayesCore::RelativeNodeAgeConstraintsReader* dmr = new RevBayesCore::RelativeNodeAgeConstraintsReader( fn.getValue(), '\t' );
    RevBayesCore::RelativeNodeAgeConstraints* dm = new RevBayesCore::RelativeNodeAgeConstraints(dmr);
    
    return new RevVariable( new RlRelativeNodeAgeConstraints(dm) );
}


/** Get argument rules */
const ArgumentRules& Func_readRelativeNodeAgeConstraints::getArgumentRules( void ) const
{
    
    static ArgumentRules argumentRules = ArgumentRules();
    static bool rulesSet = false;
    
    if (!rulesSet)
    {
        
        argumentRules.push_back( new ArgumentRule( "file", RlString::getClassTypeSpec(), "Relative or absolute name of the file.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
        rulesSet = true;
        
    }
    
    return argumentRules;
}


/** Get Rev type of object */
const std::string& Func_readRelativeNodeAgeConstraints::getClassType(void)
{
    
    static std::string revType = "Func_readRelativeNodeAgeConstraints";
    
    return revType;
}


/** Get class type spec describing type of object */
const TypeSpec& Func_readRelativeNodeAgeConstraints::getClassTypeSpec(void)
{
    
    static TypeSpec revTypeSpec = TypeSpec( getClassType(), new TypeSpec( Function::getClassTypeSpec() ) );
    
    return revTypeSpec;
}


/**
 * Get the primary Rev name for this function.
 */
std::string Func_readRelativeNodeAgeConstraints::getFunctionName( void ) const
{
    // create a name variable that is the same for all instance of this class
    std::string f_name = "readRelativeNodeAgeConstraints";
    
    return f_name;
}


/** Get type spec */
const TypeSpec& Func_readRelativeNodeAgeConstraints::getTypeSpec( void ) const
{
    
    static TypeSpec typeSpec = getClassTypeSpec();
    
    return typeSpec;
}


/** Get return type */
const TypeSpec& Func_readRelativeNodeAgeConstraints::getReturnType( void ) const
{
    
    static TypeSpec returnTypeSpec = RlRelativeNodeAgeConstraints::getClassTypeSpec();
    return returnTypeSpec;
}

