
#ifndef USE_R
#ifndef __CPURNG__
#define __CPURNG__

#include "RNG.hpp"
#include <vector>
#include "RNGParallel.hpp"

using std::vector;

template<typename RealType>
class RNGPar {

protected:

  int nrng;
  vector<RNG> r;

public:

  RNGPar();
  RNGPar(int nrng);
  RNGPar(int nrng, unsigned long seed);
  RNGPar(const RNGPar& rng);

  ExponMean<RealType> expon_mean_sampler;

  void expon_mean (RealType* samp, int nsamp, RealType* mean, int npar);
  void expon_rate (RealType* samp, int nsamp, RealType* rate, int npar);
  void chisq      (RealType* samp, int nsamp, RealType*   df, int npar);
  void norm       (RealType* samp, int nsamp, RealType*   sd, int npar);

  void norm        (RealType* samp, int nsamp, RealType*  mean, RealType*    sd, int npar);
  void gamma_scale (RealType* samp, int nsamp, RealType* shape, RealType* scale, int npar);
  void gamma_rate  (RealType* samp, int nsamp, RealType* shape, RealType*  rate, int npar);
  void igamma      (RealType* samp, int nsamp, RealType* shape, RealType* scale, int npar);
  void flat        (RealType* samp, int nsamp, RealType* lower, RealType* upper, int npar);

  void test        (RealType* samp, int nsamp, RealType* p1, int npar);

};

template<typename RealType>
RNGPar<RealType>::RNGPar() 
  : nrng(1)
  , r(1)
{
  // Do nothing.
}

template<typename RealType>
RNGPar<RealType>::RNGPar(int nrng_)
  : nrng(nrng_)
  , r(nrng_)
{
  unsigned long seed = time(NULL);
  for (int i = 0; i < nrng; i++)
    r[i].set(seed+i);
}

template<typename RealType>
RNGPar<RealType>::RNGPar(int nrng_, unsigned long seed)
  : nrng(nrng_)
  , r(nrng_)
{
  for (int i = 0; i < nrng; i++)
    r[i].set(seed+i);
}

template<typename RealType>
void RNGPar<RealType>::test(RealType* samp, int nsamp, RealType* p1, int npar)
{
  vector<RNG>* rp = &r;

  unsigned int nthreads = rp->size();
  
  int i;
  
  #pragma omp parallel shared(samp, p1, rp) private(i) num_threads(nthreads)
  {
    // // Get thread number and write out.
    // int tid = omp_get_thread_num();                                                                              
    // printf("Hello from thread %i.\n", tid); 
    
    unsigned int tid = omp_get_thread_num();
    RNG* rngp = &(rp->operator[](tid));
     
    #pragma omp for schedule(dynamic) nowait
    for (i = 0; i < nsamp; i++) {
      samp[i] = rngp -> expon_mean(p1[i % npar]);
    }
    
  }
}

#define ONEP(FNAME, CNAME)						\
  template <typename RealType>						\
  void RNGPar<RealType>:: FNAME (RealType* samp, int nsamp, RealType* p1, int npar) \
  {									\
    CNAME <RealType> sampler;						\
    draw_parallel<RealType>(samp, nsamp, p1, npar, expon_mean_sampler, &r);	\
  }									\

ONEP(expon_mean, ExponMean)
ONEP(expon_rate, ExponRate)
ONEP(chisq     ,     ChiSq)
ONEP(norm      ,     Norm1)

#undef ONEP

#define TWOP(FNAME, CNAME)						\
  template <typename RealType>						\
  void RNGPar<RealType>:: FNAME (RealType* samp, int nsamp, RealType* p1, RealType* p2, int npar) \
  {									\
    CNAME <RealType> sampler;						\
    draw_parallel<RealType>(samp, nsamp, p1, p2, npar, sampler, &r);	\
  }									\

TWOP(norm, Norm2)
TWOP(gamma_scale, GammaScale)
TWOP(gamma_rate , GammaRate)
TWOP(igamma     , IGamma)
TWOP(flat       , Flat)

#undef TWOP

#endif // __CPURNG__
#endif // check USE_R
