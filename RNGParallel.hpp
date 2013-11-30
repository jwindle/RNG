// -*- c-basic-offset: 4; -*-
// Copyright 2013 Jesse Windle - jesse.windle@gmail.com

#ifndef __RNGPARALLEL__
#define __RNGPARALLEL__

#include "RNG.hpp"
#include "omp.h"
#include <vector>

template<typename RealType>
class OneParameterSampler {
public: 
  virtual RealType draw(RealType p1, RNG& rng) = 0;
};

template<typename RealType>
class TwoParameterSampler {
public:
  virtual RealType draw(RealType p1, RealType p2, RNG& rng) = 0;
};

template<typename RealType>
void draw_parallel(RealType* samp, 
		   int nsamp, 
		   RealType* p1, 
		   int npar, 
		   OneParameterSampler<RealType>& sampler, 
		   std::vector<RNG>* rngs)
{   
    int nthreads = rngs->size();

    int i;

    #pragma omp parallel shared(samp, p1, rngs) private(i) num_threads(nthreads)
    {
      // Get thread number and write out.
      int tid = omp_get_thread_num();                                                                              
      // printf("Hello from thread %i.\n", tid); 

      // RNG* rngp = &(rngs->operator[](tid));
      
      #pragma omp for schedule(dynamic) nowait
      for (i = 0; i < nsamp; i++) {
	  samp[i] = sampler.draw(p1[i % npar], (*rngs)[tid]);
	  // samp[i] = (*rngs)[tid].gamma_rate(p1[i % npar], 1.0);
      }
      
    }
    
}

template<typename RealType>
void draw_parallel(RealType* samp, 
		   int nsamp, 
		   RealType* p1, 
		   RealType* p2,
		   int npar, 
		   TwoParameterSampler<RealType>& sampler, 
		   std::vector<RNG>* rngs)
{   
    int nthreads = rngs->size();

    int i;

    #pragma omp parallel shared(samp, p1, rngs) private(i) num_threads(nthreads)
    {
      // Get thread number and write out.
      int tid = omp_get_thread_num();                                                                              
      // printf("Hello from thread %i.\n", tid); 

      RNG* rngp = &(rngs->operator[](tid));
      
      #pragma omp for schedule(dynamic) nowait
      for (i = 0; i < nsamp; i++) {
	samp[i] = sampler.draw(p1[i % npar], p2[i % npar], *rngp);
      }
      
    }
    
}

#define ONEP(NAME, FUNC, P1)						\
  template<typename RealType>						\
  class NAME : public OneParameterSampler<RealType> {			\
  public:								\
    RealType draw(RealType p1, RNG& rng) { return rng.FUNC((double)p1); } \
  };									\
  
ONEP(ExponMean, expon_mean, mean)
ONEP(ExponRate, expon_rate, rate)
ONEP(ChiSq    , chisq     , df  )
ONEP(Norm1    , norm      , sd  )

#undef ONEP

#define TWOP(NAME, FUNC, P1, P2)					\
  template<typename RealType>						\
  class NAME : public TwoParameterSampler<RealType> {			\
  public:								\
    RealType draw(RealType p1, RealType p2, RNG& rng) { return rng.FUNC((double)p1, (double)p2); } \
  };									\
  
TWOP(Norm2     , norm       ,  mean,     sd)
TWOP(GammaScale, gamma_scale, shape,  scale)
TWOP(GammaRate , gamma_rate , shape,  scale)
TWOP(IGamma    , igamma     , shape,  scale)
TWOP(Flat      , flat       ,     a,      b)

#undef TWOP

#endif
