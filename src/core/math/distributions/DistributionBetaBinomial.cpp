/**
 * @file DistributionBetaBinomial
 * This file contains the functions of the beta-binomial distribution.
 *
 * @brief Implementation of the beta binomial distribution.
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes core development team (JK, WD, WP) 15.11.2017
 * @license GPL version 3
 * @version 1.0
 * @since 2011-03-17, version 1.0
 *
 * $Id$
 */

#include <cmath>
#include "DistributionBeta.h"
#include "DistributionBinomial.h"
#include "DistributionNormal.h"
#include "RbConstants.h"
#include "RbException.h"
#include "RbMathFunctions.h"
#include "RbMathCombinatorialFunctions.h"
#include "RbMathHelper.h"
#include "RbMathLogic.h"
#include "DistributionBetaBinomial.h"

using namespace RevBayesCore;

/*!
 * This function calculates the probability density 
 * for a beta-binomially-distributed random variable.
 * The beta-binomial distribution is the binomial distribution
 * in which the probability of successes at each trial is random,
 * and follows a beta distribution.
 *
 * \brief Beta Binomial probability density.
 * \param n is the number of trials. 
 * \param p is the success probability. 
 * \param x is the number of successes. 
 * \return Returns the probability density.
 * \throws Does not throw an error.
 */
double RbStatistics::BetaBinomial::cdf(double n, double a, double b, double x)
{
	throw RbException("The Beta Binomial cdf is not yet implemented in RB.");
}

/*!
 * This function draws a random variable
 * from a beta-binomial distribution.
 *
 * From R:
 *
 *     (1) pdf() has both p and q arguments, when one may be represented
 *         more accurately than the other (in particular, in df()).
 *     (2) pdf() does NOT check that inputs x and n are integers. This
 *         should be done in the calling function, where necessary.
 *         -- but is not the case at all when called e.g., from df() or dbeta() !
 *     (3) Also does not check for 0 <= p <= 1 and 0 <= q <= 1 or NaN's.
 *         Do this in the calling function.
 *
 *  REFERENCE
 *
 *	Kachitvichyanukul, V. and Schmeiser, B. W. (1988).
 *	Binomial random variate generation.
 *	Communications of the ACM 31, 216-222.
 *	(Algorithm BTPEC).
 *
 *
 * \brief Beta-Binomial probability density.
 * \param n is the number of trials.
 * \param pp is the success probability.
 * \param a is the number of successes. ????
 * \param b is the ????
 * \return Returns the probability density.
 * \throws Does not throw an error.
 */


int RbStatistics::BetaBinomial::rv(double n, double a, double b, RevBayesCore::RandomNumberGenerator &rng)
{
	int y;

	double p = RbStatistics::Beta::rv(a,b,rng);
	y = RbStatistics::Binomial::rv(n,p,rng);
	return y;
}

/*!
 * This function calculates the probability density 
 * for a beta-binomially-distributed random variable.
 *
 * \brief Beta-Binomial probability density.
 * \param n is the number of trials. 
 * \param p is the success probability. 
 * \param x is the number of successes. 
 * \return Returns the probability density.
 * \throws Does not throw an error.
 */
double RbStatistics::BetaBinomial::lnPdf(double n, double a, double b, double value) {

    return pdf(value, n, a, b, true);
}

/*!
 * This function calculates the probability density 
 * for a beta-binomially-distributed random variable.
 *
 * \brief Beta-Binomial probability density.
 * \param n is the number of trials. 
 * \param y is the number of successes.
 * \param a is the alpha parameter for the beta distribution
 * \param b is the beta parameter for the beta distribution
 * \return Returns the probability density.
 * \throws Does not throw an error.
 */


double RbStatistics::BetaBinomial::pdf(double y, double n, double a, double b, bool asLog)
{

    double constant;
    if (a==0)
    		return((y == 0) ? (asLog ? 0.0 : 1.0) : (asLog ? RbConstants::Double::neginf : 0.0) );
    		//return constant;

    if (b==0)
    		return((y == n) ? (asLog ? 0.0 : 1.0) : (asLog ? RbConstants::Double::neginf : 0.0) );
    		//return constant;

    constant = RevBayesCore::RbMath::lnChoose(n, y);


    	double prUnnorm = constant + RbStatistics::Beta::lnPdf(a, b, y);
    	double prNormed = prUnnorm - RbStatistics::Beta::lnPdf(a, b, y);

    	if(asLog == false)
    		return exp(prNormed);
    	else
    		return prNormed;
}


/*!
 * This function calculates the probability density 
 * for a beta-binomially-distributed random variable.
 *
 * From R:
 *
 *     (1) pdf() has both p and q arguments, when one may be represented
 *         more accurately than the other (in particular, in df()).
 *     (2) pdf() does NOT check that inputs x and n are integers. This
 *         should be done in the calling function, where necessary.
 *         -- but is not the case at all when called e.g., from df() or dbeta() !
 *     (3) Also does not check for 0 <= p <= 1 and 0 <= q <= 1 or NaN's.
 *         Do this in the calling function.
 *
 * \brief Beta Binomial probability density.
 * \param n is the number of trials. 
 * \param a is the alpha parameter.
 * \param b is the beta parameter.
 * \return Returns the probability density.
 * \throws Does not throw an error.
 */

//double RbStatistics::BetaBinomial::quantile(double quantile_prob, double n, double p)
double quantile(double quantile_prob, double n, double a, double b)
{
	throw RbException("There is no simple formula for this, and it is not yet implemented in RB.");
}
