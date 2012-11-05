// Copyright 2012 Jesse Windle - jwindle@ices.utexas.edu

// This program is free software: you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

/*********************************************************************

  This class wraps GSL's random number generator and random
  distribution functions into a class.  We use the Mersenne Twister
  for random number generation since it has a large period, which is
  what we want for MCMC simulation.

  When compiling include -lgsl -lcblas -llapack .

*********************************************************************/

#ifndef __BASICRNG__
#define __BASICRNG__

#include <stdio.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_sf.h>

#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <cmath>

using std::string;
using std::ofstream;
using std::ifstream;

//////////////////////////////////////////////////////////////////////
			      // RNG //
//////////////////////////////////////////////////////////////////////

class BasicRNG {

 protected:

  gsl_rng * r;

 public:

  // Constructors and destructors.
  BasicRNG();
  BasicRNG(unsigned long seed);
  BasicRNG(const BasicRNG& rng);

  ~BasicRNG()
    { gsl_rng_free (r); }

  // Assignment=
  inline BasicRNG& operator=(const BasicRNG& rng);

  // Read / Write / Set
  bool read (const string& filename);
  bool write(const string& filename);
  void set(unsigned long seed);

  // Get rng -- be careful.  Needed for other random variates.
  gsl_rng* getrng() { return r; }

  // Random variates.
  inline double unif  ();                             // Uniform
  inline double expon_mean(double mean);                  // Exponential
  inline double expon_rate(double rate);                  // Exponential
  inline double chisq (double df);                    // Chisq
  inline double norm  (double sd);                    // Normal
  inline double norm  (double mean , double sd);      // Normal
  inline double gamma_scale (double shape, double scale); // Gamma_Scale
  inline double gamma_rate  (double shape, double rate);  // Gamma_Rate
  inline double igamma(double shape, double scale);   // Inv-Gamma
  inline double flat  (double a=0  , double b=1  );   // Flat
  inline double beta  (double a=1.0, double b=1.0);   // Beta

  inline int bern  (double p);                     // Bernoulli

  // CDF
  static inline double p_norm (double x, int use_log=0);
  static inline double p_gamma_rate(double x, double shape, double rate, int use_log=0);

  // Density
  static inline double d_beta(double x, double a, double b);

  // Utility
  static inline double Gamma (double x, int use_log=0);

}; // BasicRNG

//////////////////////////////////////////////////////////////////////
			  // Constructors //
//////////////////////////////////////////////////////////////////////

BasicRNG::BasicRNG()
{
  r = gsl_rng_alloc(gsl_rng_mt19937);
  gsl_rng_set (r, time(NULL));
}

BasicRNG::BasicRNG(unsigned long seed)
{
  r = gsl_rng_alloc(gsl_rng_mt19937);
  gsl_rng_set (r, seed);
}

BasicRNG::BasicRNG(const BasicRNG& rng)
{
  r = gsl_rng_alloc(gsl_rng_mt19937);
  gsl_rng_memcpy(r, rng.r );
}

//////////////////////////////////////////////////////////////////////
			  // Assignment= //
//////////////////////////////////////////////////////////////////////

inline BasicRNG& BasicRNG::operator=(const BasicRNG& rng)
{
  // The random number generators must be of the same type.
  gsl_rng_memcpy(r, rng.r );
  return *this;
}

//////////////////////////////////////////////////////////////////////
			  // Read / Write //
//////////////////////////////////////////////////////////////////////


bool BasicRNG::read(const string& filename)
{

  FILE *file;
  file = fopen(filename.c_str(), "r");
  if (file==NULL) return false;
  // Must be initialized to same type.  gsl_rng_free(r);
  int success = gsl_rng_fread(file, r);
  fclose(file);
  return success==0;
} // Read

bool BasicRNG::write(const string& filename){
  FILE *file;
  file = fopen(filename.c_str(), "w");
  int success = gsl_rng_fwrite(file, r);
  fclose(file);
  return success==0;
} // Write

void BasicRNG::set(unsigned long seed)
{
  gsl_rng_set(r, seed);
} // Set

//////////////////////////////////////////////////////////////////////
		      // GSL Random Variates //
//////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------
// Distributions with one parameter.

#define ONEP(NAME, CALL, P1)			\
  inline double BasicRNG::NAME(double P1)	\
  {						\
    return CALL (r, P1);			\
  }						\

ONEP(expon_mean, gsl_ran_exponential, mean)
ONEP(chisq,  gsl_ran_chisq      , df  )
ONEP(norm,   gsl_ran_gaussian   , sd  )

#undef ONEP

//--------------------------------------------------------------------
// Distributions with two parameters.

#define TWOP(NAME, CALL, P1, P2)			\
  inline double BasicRNG::NAME(double P1, double P2)	\
  {							\
    return CALL (r, P1, P2);				\
  }							\

TWOP(gamma_scale, gsl_ran_gamma, shape, scale)
TWOP(flat , gsl_ran_flat , a    , b    )
TWOP(beta , gsl_ran_beta , a    , b    )

// x ~ Gamma(shape=a, scale=b)
// x ~ x^{a-1} exp(x / b).

#undef TWOP

//////////////////////////////////////////////////////////////////////
		     // Custom Random Variates //
//////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------
			   // Bernoulli //

inline int BasicRNG::bern(double p)
{
  return gsl_ran_bernoulli(r, p);
}

//--------------------------------------------------------------------
			    // Uniform //

inline double BasicRNG::unif()
{
  return gsl_rng_uniform(r);
} // unif

//--------------------------------------------------------------------
			  // Exponential //
inline double BasicRNG::expon_rate(double rate)
{
  return expon_mean(1.0 / rate);
}

//--------------------------------------------------------------------
			    // Normal //

inline double BasicRNG::norm(double mean, double sd)
{
  return mean + gsl_ran_gaussian(r, sd);
} // norm

//--------------------------------------------------------------------
			   // Gamma_Rate //

inline double BasicRNG::gamma_rate(double shape, double rate)
{
  return gamma_scale(shape, 1.0 / rate);
}

//--------------------------------------------------------------------
			   // Inv-Gamma //

// a = shape, b = scale
// x ~ IG(shape, scale) ~ x^{-a-1} exp(b / x).
// => 1/x ~ Ga(shape, 1/scale).

inline double BasicRNG::igamma(double shape, double scale)
{
  return 1.0/gsl_ran_gamma_knuth(r, shape, 1.0/scale);
} // igamma

////////////////////////////////////////////////////////////////////////////////

inline double BasicRNG::p_norm(double x, int use_log)
{
  double m = gsl_cdf_ugaussian_P(x);
  if (use_log) m = log(m);
  return m;
}

inline double BasicRNG::p_gamma_rate(double x, double shape, double rate, int use_log)
{
  double scale = 1.0 / rate;
  double y = gsl_cdf_gamma_P(x, shape, scale);
  if (use_log) y = log(y);
  return y;
}

////////////////////////////////////////////////////////////////////////////////

inline double BasicRNG::Gamma(double x, int use_log)
{
  double y = gsl_sf_lngamma(x);
  if (!use_log) y = exp(y);
  return y;
}

////////////////////////////////////////////////////////////////////////////////

inline double BasicRNG::d_beta(double x, double a, double b)
{
  return gsl_ran_beta_pdf(x, a, b);
}

////////////////////////////////////////////////////////////////////////////////

#endif
