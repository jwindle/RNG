// Copyright 2012 Jesse Windle - jwindle@ices.utexas.edu

// This program is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

//////////////////////////////////////////////////////////////////////

#ifndef __RNG__
#define __RNG__

#include <stdio.h>
#include <stdexcept>
#include <cmath>

#ifdef USE_R
#include "RRNG.hpp"
#else
#include "GRNG.hpp"
#endif

// #ifndef __MYMAT__
// #define __MYMAT__
// typedef MatrixFrame MyMat;
// #endif

const double SQRT2PI = 2.50662827;

class RNG : public BasicRNG {

  // Truncated Normal Helper Functions / Variables.
  double alphastar(double left);
  double lowerbound(double left);

  // Truncated Right Gamma Helper Functions.
  double omega_k(int k, double a, double b);

 public:

  // Random variates.  I need to do this so I can overload the function names.
  using BasicRNG::unif;
  using BasicRNG::expon_mean;
  using BasicRNG::expon_rate;
  using BasicRNG::chisq ;
  using BasicRNG::norm  ;
  using BasicRNG::gamma_scale ;
  using BasicRNG::gamma_rate  ;
  using BasicRNG::igamma;
  using BasicRNG::flat  ;
  using BasicRNG::beta  ;

  using BasicRNG::p_norm;
  using BasicRNG::p_gamma_rate;
  
  using BasicRNG::Gamma;

  double Beta(double a, double b, bool log=false);

  // Truncated Normal
  double tnorm(double left);               // One sided standard.
  double tnorm(double left, double right); // Two sided standard.
  double tnorm(double left, double mu, double sd);
  double tnorm(double left, double right, double mu, double sd);

  // Right tail of normal
  double tnorm_tail(double t);

  // Right truncated gamma.
  double right_tgamma_reject(double shape, double rate);
  double right_tgamma_beta(double shape, double rate);
  double rtgamma_rate(double shape, double rate, double right);

  // Random variates with Mat.  Fills the Mat with samples.
  template<typename Mat> void unif  (Mat& M);
  template<typename Mat> void expon_mean(Mat& M, double mean);
  template<typename Mat> void expon_rate (Mat& M, double rate);
  template<typename Mat> void chisq (Mat& M, double df);
  template<typename Mat> void norm  (Mat& M, double sd);
  template<typename Mat> void norm  (Mat& M, double mean , double sd);
  template<typename Mat> void gamma_scale (Mat& M, double shape, double scale);
  template<typename Mat> void gamma_rate  (Mat& M, double shape, double scale);
  template<typename Mat> void igamma(Mat& M, double shape, double scale);
  template<typename Mat> void flat  (Mat& M, double a    , double b    );

  template<typename Mat> void expon_mean(Mat& M, const Mat& mean);
  template<typename Mat> void expon_rate (Mat& M, const Mat& rate);
  template<typename Mat> void chisq (Mat& M, const Mat& df);
  template<typename Mat> void norm  (Mat& M, const Mat& sd);
  template<typename Mat> void norm  (Mat& M, const Mat& mean , const Mat& sd);
  template<typename Mat> void gamma_scale (Mat& M, const Mat& shape, const Mat& scale);
  template<typename Mat> void gamma_rate  (Mat& M, const Mat& shape, const Mat& scale);
  template<typename Mat> void igamma(Mat& M, const Mat& shape, const Mat& scale);
  template<typename Mat> void flat  (Mat& M, const Mat& a    , const Mat& b);

}; // RNG

////////////////////////////////////////////////////////////////////////////////
			   // BASIC RANDOM VARIATE //
////////////////////////////////////////////////////////////////////////////////

template<typename Mat>
void RNG::unif(Mat& M)
{
  for(int i = 0; i < M.size(); ++i)
    M(i) = BasicRNG::flat();
} // unif

#define ONEP(NAME, P1)				\
  template<typename Mat>			\
  void RNG::NAME(Mat& M, double P1)		\
  {						\
    for(int i = 0; i < M.size(); i++)		\
      M(i) = NAME (P1);				\
  }						\
  template<typename Mat>			\
  void RNG::NAME(Mat& M, const Mat& P1)		\
  {						\
    for(int i = 0; i < M.size(); i++)		\
      M(i) = NAME (P1(i % P1.size()));		\
  }						\

ONEP(expon_mean, mean)
ONEP(expon_rate, rate)
ONEP(chisq     ,   df)
ONEP(norm      ,   sd)

#undef ONEP

//--------------------------------------------------------------------
// Distributions with two parameters.

#define TWOP(NAME, P1, P2)					\
  template<typename Mat>					\
  void RNG::NAME(Mat& M, double P1, double P2)			\
  {								\
    for(int i = 0; i < M.size(); i++)				\
      M(i) = NAME (P1, P2);					\
  }								\
  template<typename Mat>					\
  void RNG::NAME(Mat& M, const Mat& P1, const Mat& P2)		\
  {								\
    int p1len = P1.size();					\
    int p2len = P2.size();					\
    for(int i = 0; i < M.size(); i++)				\
      M(i) = NAME (P1(i%p1len), P2(i%p2len) );			\
  }								\

TWOP(norm       ,  mean,  sd)
TWOP(gamma_scale, shape,  scale)
TWOP(gamma_rate , shape,  scale)
TWOP(igamma     , shape,  scale)
TWOP(flat       ,     a,  b    )

#undef TWOP

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
	       // TRUNCATED NORMAL HELPER FUNCTIONS //
//////////////////////////////////////////////////////////////////////

double RNG::alphastar(double left)
{
  return 0.5 * (left + sqrt(left + 4));
} // alphastar

double RNG::lowerbound(double left)
{
  double astar  = alphastar(left);
  double lbound = left + exp(0.5 * left * (left - astar) + 0.5);
  return lbound;
} // lowerbound

//////////////////////////////////////////////////////////////////////
		     // DRAW TRUNCATED NORMAL //
//////////////////////////////////////////////////////////////////////

double RNG::tnorm(double left)
{
  double rho, ppsl;

  if (left < 0) { // Accept/Reject Normal
    while (true) {
      ppsl = norm(0.0, 1.0);
      if (ppsl > left) return ppsl;
    }
  }
  else { // Accept/Reject Exponential
    double astar = alphastar(left);
    while (true) {
      ppsl = expon_rate(astar) + left;
      rho  = exp( -0.5 * (ppsl - astar) * (ppsl - astar) );
      if (unif() < rho) return ppsl;
    }
  }
} // tnorm
//--------------------------------------------------------------------

double RNG::tnorm(double left, double right)
{
  // The most difficult part of this algorithm is figuring out all the
  // various cases.  An outline is summarized in the Appendix.

  // Check input.
  if (std::isnan(right) || std::isnan(left)) {
    fprintf(stderr, "Warning: nan sent to RNG::tnorm: left=%g, right=%g\n", left, right);
    throw std::runtime_error("RNG::tnorm: parameter problem.\n");
  }

  if (right < left) {
    fprintf(stderr, "Warning: left: %g, right:%g.\n", left, right);
    throw std::runtime_error("RNG::tnorm: parameter problem.\n");
  }

  double rho, ppsl;

  if (left >= 0) {
    double lbound = lowerbound(left);
    if (right > lbound) { // Truncated Exponential.
      double astar = alphastar(left);
      while (true) {
	do
	  ppsl = expon_rate(astar) + left;
	while(ppsl > right);
	// REVIEW REVIEW - TAKE ANOTHER LOOK AT THIS.
	rho  = exp(-0.5*(ppsl - astar)*(ppsl-astar));
	if (unif() < rho) return ppsl;
	// if (ppsl < right) return ppsl;
      }
    }
    else {
      while (true) {
	ppsl = flat(left, right);
	rho  = exp(0.5 * (left*left - ppsl*ppsl));
	if (unif() < rho) return ppsl;
      }
    }
  }
  else if (right >= 0) {
    if ( (right - left) < SQRT2PI ){
      while (true) {
	ppsl = flat(left, right);
	rho  = exp(-0.5 * ppsl * ppsl);
	if (unif() < rho) return ppsl;
      }
    }
    else{
      while (true) {
	ppsl = norm(0, 1);
	if (left < ppsl && ppsl < right) return ppsl;
      }
    }
  }
  else {
    return -1. * tnorm(-1.0 * right, -1.0 * left);
  }
} // tnorm
//--------------------------------------------------------------------

double RNG::tnorm(double left, double mu, double sd)
{
  double newleft = (left - mu) / sd;
  return mu + tnorm(newleft) * sd;
} // tnorm
//--------------------------------------------------------------------

double RNG::tnorm(double left, double right, double mu, double sd)
{
  double newleft  = (left - mu) / sd;
  double newright = (right - mu) / sd;

  // I want to check this here as well so we can see what the input was.
  // It may be more elegant to try and catch tdraw.
  if (newright < newleft) {
    fprintf(stderr, "left, right, mu, sd: %g, %g, %g, %g \n", left, right, mu, sd);
    fprintf(stderr, "nleft, nright: %g, %g\n", newleft, newright);
    throw std::runtime_error("RNG::tnorm: parameter problem.\n");
  }

  double tdraw = tnorm(newleft, newright);
  double draw = mu + tdraw * sd;

  // It may be the case that there is some numerical error and that the draw
  // ends up out of bounds.
  if (draw < left || draw > right){
    fprintf(stderr, "Error in tnorm: draw not in bounds.\n");
    fprintf(stderr, "left, right, mu, sd: %g, %g, %g, %g\n", left, right, mu, sd);
    fprintf(stderr, "nleft, nright, tdraw, draw: %g, %g, %g, %g\n", newleft, newright, tdraw, draw);
  }

  return draw;
} // tnorm
//--------------------------------------------------------------------

// Right tail of normal by Devroye
//------------------------------------------------------------------------------
double RNG::tnorm_tail(double t)
{
  double E1 = expon_rate(1.0);
  double E2 = expon_rate(1.0);
  while ( E1*E1 > 2 * E2 / t) {
    E1 = expon_rate(1.0);
    E2 = expon_rate(1.0);
  }
  return (1 + t * E1) / sqrt(t);
}

//------------------------------------------------------------------------------

// Truncatatoin at t = 1.
inline double RNG::right_tgamma_reject(double shape, double rate)
{
  double x = 2.0;
  while (x > 1.0) 
    x = gamma_rate(shape, rate);
  return x;
}

double RNG::omega_k(int k, double a, double b)
{
  double log_coef = -b + (a+k-1) * log(b) - Gamma(a+k, true) - p_gamma_rate(1.0, a, b, true);
  return exp(log_coef);
}

// Truncation at t = 1.
double RNG::right_tgamma_beta(double shape, double rate)
{
  double a = shape;
  double b = rate;

  double u = unif();

  int k = 1;
  double cdf = omega_k(1, a, b);
  while (u > cdf) {
    cdf += omega_k(++k, a, b);
    if (k % 100000 == 0) {
      printf("right_tgamma_beta (itr k=%i): a=%g, b=%g, u=%g, cdf=%g\n", k, a, b, u, cdf);
      #ifdef USE_R
      R_CheckUserInterrupt();
      #endif
    }
  }

  return beta(a, k);
}

double RNG::rtgamma_rate(double shape, double rate, double right_t)
{
  // x \sim (a,b,t)
  // ty = x
  // y \sim (a, bt, 1);
  double a = shape;
  double b = rate * right_t;
  
  double p = p_gamma_rate(1, a, b);
  double y = 0.0;
  if (p > 0.95)
    y = right_tgamma_reject(a, b);
  else
    y = right_tgamma_beta(a,b);
  
  double x = right_t * y;
  return x;
}

//------------------------------------------------------------------------------

double RNG::Beta(double a, double b, bool log)
{
  double out = Gamma(a, true) + Gamma(b, true) - Gamma(a+b,true);
  if (!log) out = exp(out);
  return out;
}

#endif
