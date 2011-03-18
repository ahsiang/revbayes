/**
 * @file
 * This file contains the implementation of ArgumentRule, which is
 * the base class for objects used to describe rules for arguments
 * passed to functions.
 *
 * @brief Implementation of ArgumentRule 
 *
 * (c) Copyright 2009-
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-11-20, version 1.0
 *
 * $Id$
 */

#include "ArgumentRule.h"
#include "DAGNode.h"
#include "RbException.h"
#include "RbNames.h"
#include "RbObject.h"
#include "VectorString.h"
#include "Workspace.h"

#include <sstream>


/** Construct rule based on default value; use "" for no label. */
ArgumentRule::ArgumentRule(const std::string& argName, RbObject* defVal)
    : RbInternal(), label(argName), argSlot(defVal->getType(), defVal->getDim()), hasDefaultVal(true) {
}


/** Construct rule without default value; use "" for no label. */
ArgumentRule::ArgumentRule(const std::string& argName, const TypeSpec& argTypeSp)
    : RbInternal(), label(argName), argSlot(argTypeSp), hasDefaultVal(false) {
}


/** Construct rule with default value */
ArgumentRule::ArgumentRule(const std::string& argName, const TypeSpec& argTypeSp, RbObject* defValue)
    : RbInternal(), label(argName), argSlot(argTypeSp), hasDefaultVal(true) {

    argSlot.setValue( defValue );
}


/** Construct rule with default reference or value variable. */
ArgumentRule::ArgumentRule(const std::string& argName, const TypeSpec& argTypeSp, DAGNode* defVariable)
    : RbInternal(), label(argName), argSlot(argTypeSp), hasDefaultVal(true) {

    if ( argSlot.getTypeSpec().isReference() )
        argSlot.setReference( defVariable );
    else
        argSlot.setValue( defVariable );
}


/** Get class vector describing type of object */
const VectorString& ArgumentRule::getClass(void) const { 

    static VectorString rbClass = VectorString(ArgumentRule_name) + RbInternal::getClass();
	return rbClass; 
}


/** Get default value */
RbObject* ArgumentRule::getDefaultValue(void) const {

    if ( !hasDefault() )
        throw RbException("There is no default value for argument '" + label + "'");

    const RbObject* defValue = argSlot.getValue();
    if ( defValue == NULL )
        return NULL;
    else
        return defValue->clone();
}


/** Get default variable */
DAGNode* ArgumentRule::getDefaultVariable(void) const {

    if ( !hasDefault() )
        throw RbException("Missing default value");

    const DAGNode* defValue = argSlot.getVariable();
    if ( defValue == NULL )
        return NULL;
    else
        return defValue->clone();
}


/** Get default reference (a variable ptr corresponding to a '&' argument) */
DAGNode* ArgumentRule::getDefaultReference(void) {

    if ( !hasDefault() )
        throw RbException("Missing default value");

    return argSlot.getReference();
}


/** Test if argument is valid; for consistency, we also evaluate the argument here */
bool ArgumentRule::isArgValid(DAGNode* var) const {
    
    if ( var == NULL )
        return true;

    // This call causes the var to be evaluated, so it could have side effects
    const RbObject* value = var->getValue();

    if ( Workspace::userWorkspace().isXOfTypeY( var->getValueType(), argSlot.getTypeSpec().getType() ) == true &&
         var->getDim() == argSlot.getTypeSpec().getDim()  )
        return true;

    if ( value->isConvertibleTo( argSlot.getTypeSpec().getType(), argSlot.getTypeSpec().getDim() ) == true)
        return true;

    return false;
}


/** Print value for user (in descriptions of functions, for instance) */
void ArgumentRule::printValue(std::ostream &o) const {

    o << "<" << argSlot.getTypeSpec() << ">";
    o << " \"" << label << "\"";
    if ( hasDefaultVal ) {
        o << " = ";
        if ( isReference() ) {
            if ( argSlot.getReference() == NULL )
                o << "NULL";
            else
                argSlot.getReference()->printValue( o );
        }
        else {
            if ( argSlot.getVariable() == NULL )
                o << "NULL";
            else
                argSlot.getVariable()->printValue( o );
        }
    }
}


/** Provide complete information about object */
std::string ArgumentRule::toString(void) const {

    std::ostringstream o;

    o << "ArgumentRule:" << std::endl;
    o << "label         = " << label << std::endl;
    o << "argSlot       = " << argSlot << std::endl;
    o << "hasDefaultVal = " << hasDefaultVal << std::endl;

    return o.str();
}

