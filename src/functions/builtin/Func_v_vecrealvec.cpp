/**
 * @file
 * This file contains the declaration of Func_v_vecrealvec, which 
 * constructs a MatrixReal from a vector of VectorReals.
 *
 * @brief Implementation of Func_v_vecrealvec
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @package functions
 * @since Version 1.0, 2009-09-03
 *
 * $Id$
 */


#include "ArgumentRule.h"
#include "ContainerNode.h"
#include "DAGNode.h"
#include "DeterministicNode.h"
#include "Ellipsis.h"
#include "Func_v_vecrealvec.h"
#include "MatrixReal.h"
#include "RbException.h"
#include "Real.h"
#include "RbNames.h"
#include "StochasticNode.h"
#include "TypeSpec.h"
#include "VectorString.h"
#include "VectorReal.h"

#include <cassert>
#include <cmath>

/** Clone object */
Func_v_vecrealvec* Func_v_vecrealvec::clone(void) const {

    return new Func_v_vecrealvec(*this);
}


/** Execute function */
DAGNode* Func_v_vecrealvec::executeOperation(const std::vector<VariableSlot>& args) {

    // Get the dimensions
    int numCols = ((VectorReal*)(args[0].getValue()))->size();
    int numRows = args.size();

    // set up a double matrix
    std::vector<std::vector<double> > tmpM;
    tmpM.resize(numRows);
    for (size_t i=0; i<tmpM.size(); i++)
        tmpM[i].resize(numCols);
    std::vector<double> tmpV;
            
    // set the matrix, row-by-row
    for ( size_t i = 0; i < args.size(); i++ ) {
        
        tmpV = ((VectorReal*)(args[0].getValue()))->getValue();
        
        if (tmpV.size() != numCols)
            throw RbException("The number of entries in each row of the matrix must be equal");
        
        tmpM[0] = tmpV;
    }
        
    // Make and return the matrix
    return new ContainerNode(new MatrixReal(tmpM));
}


/** Get argument rules */
const ArgumentRules& Func_v_vecrealvec::getArgumentRules(void) const {

    static ArgumentRules argumentRules;
    static bool          rulesSet = false;

    if (!rulesSet) 
		{
        argumentRules.push_back(new ArgumentRule("", VectorReal_name));
        argumentRules.push_back(new ArgumentRule("", VectorReal_name));
        argumentRules.push_back(new Ellipsis(VectorReal_name));
        rulesSet = true;
		}

    return argumentRules;
}


/** Get class vector describing type of object */
const VectorString& Func_v_vecrealvec::getClass(void) const {

    static VectorString rbClass = VectorString(Func_v_vecrealvec_name) + RbFunction::getClass();
    return rbClass;
}


/** Get return type */
const TypeSpec Func_v_vecrealvec::getReturnType(void) const {

    return TypeSpec(MatrixReal_name);
}

