#include "Matrix.h"
#include "RNG.hpp"
#include <iostream>

using std::cout;

int main(int argc, char** argv)
{
  RNG r;

  int M = 100;

  Matrix samp(M);

  //for(int i = 0; i < M; i++)
  //  cout << r.tnorm(0.5) << "\n";

  // Normal
  r.norm(samp, 0.0, 1.0);
  samp.dump("norm.txt", false);

  // Gamma scale
  r.gamma_scale(samp, 0.5, 0.5);
  samp.dump("gamma_scale.txt", false);

  // Gamma rate
  r.gamma_rate(samp, 0.5, 0.5);
  samp.dump("gamma_rate.txt", false);

  // Inverse gamma
  r.igamma(samp, 4.0, 3.0);
  samp.dump("igamma.txt", false);

  // Truncated normal
  r.tnorm(samp, 1.0, 1.0, 0.0, 1.0);
  samp.dump("tnorm.txt", false);

  cout << RNG::p_norm(-1.96) << " " << RNG::p_norm(0.0) << "\n";

  r.unif(samp);

  return 0;
}
