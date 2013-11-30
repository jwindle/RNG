
#include "RNG.hpp"
#include "RNGParallel.hpp"
#include "CPURNG.hpp"
#include <vector>
#include <time.h>
#include <sys/time.h>

using std::vector;

double calculateSeconds(const timeval &time1, const timeval &time2) {
    return time2.tv_sec - time1.tv_sec + (double)(time2.tv_usec - time1.tv_usec) / 1000000.0;
}

void testRNGPar() {

 typedef double Real;

  int nsamp   = 2000000;
  int npar    = 1000;
  int reps    = 10;
  int nthread =  3;

  vector<RNG>  rngs(nthread);
  
  vector<Real> samp(nsamp);
  vector<Real> p1(npar);
  vector<Real> p2(npar);

  fill(p1.begin(), p1.end(), 4.2);
  fill(p2.begin(), p2.end(), 2.0);

  // ExponMean<double> sampler;
  Norm1<double> sampler;

  RNGPar<double> r(nthread);

  struct timeval start, stop;
  gettimeofday(&start, NULL);

  for (int i=0; i<reps; i++) {

    draw_parallel(&samp[0], nsamp, &p1[0], npar, sampler, &rngs);

    // r.test(&samp[0], nsamp, &p1[0], npar);
    // r.expon_rate(&samp[0], nsamp, &p1[0], npar);
    // r.chisq     (&samp[0], nsamp, &p1[0], npar);
    // r.norm      (&samp[0], nsamp, &p1[0], npar);
    
    // r.norm       (&samp[0], nsamp, &p1[0], &p2[0], npar);
    // r.gamma_scale(&samp[0], nsamp, &p1[0], &p2[0], npar);
    // r.gamma_rate (&samp[0], nsamp, &p1[0], &p2[0], npar);
    // r.igamma     (&samp[0], nsamp, &p1[0], &p2[0], npar);
    // r.flat       (&samp[0], nsamp, &p1[0], &p2[0], npar);

  }

  gettimeofday(&stop, NULL);

  double diff = calculateSeconds(start, stop);

  printf("Time: %g\n", diff);

}

int main() {

  testRNGPar();

}
