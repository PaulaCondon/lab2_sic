#pragma once
#include <math.h>

namespace TinyIA {
class RedNeuronal {
public:
  int predict(const float entrada[4]) const {
  const float media[4] = {54.6052941f, 370.039216f, 23.5911765f, 64.4803922f};
  const float escala[4] = {36.8399656f, 32.3136957f, 0.140798104f, 0.957226305f};
  float x[4];
  for (int i=0; i<4; ++i) x[i] = (entrada[i]-media[i])/escala[i];
  const float w0[4] = {-1.84859678f, -0.78648048f, 0.604029988f, 0.369584093f};
  float h0 = tanhf(x[0]*w0[0]+x[1]*w0[1]+x[2]*w0[2]+x[3]*w0[3]+1.17195292f);
  const float w1[4] = {2.3614307f, 0.964446246f, -0.802069291f, -0.548088545f};
  float h1 = tanhf(x[0]*w1[0]+x[1]*w1[1]+x[2]*w1[2]+x[3]*w1[3]+-1.46278434f);
  const float w2[4] = {2.3313416f, 0.935501856f, -0.664800041f, -0.399629993f};
  float h2 = tanhf(x[0]*w2[0]+x[1]*w2[1]+x[2]*w2[2]+x[3]*w2[3]+-1.61049657f);
  const float w3[4] = {-0.0674696929f, -0.00972585309f, -0.0505723265f, -0.0560660767f};
  float h3 = tanhf(x[0]*w3[0]+x[1]*w3[1]+x[2]*w3[2]+x[3]*w3[3]+-0.0150922476f);
  const float w4[4] = {-2.38343186f, -0.991000497f, 0.573391456f, 0.374693084f};
  float h4 = tanhf(x[0]*w4[0]+x[1]*w4[1]+x[2]*w4[2]+x[3]*w4[3]+1.74362043f);
  const float w5[4] = {-1.36780156f, -0.459793099f, 0.536868501f, 0.33932946f};
  float h5 = tanhf(x[0]*w5[0]+x[1]*w5[1]+x[2]*w5[2]+x[3]*w5[3]+0.802219428f);
  float z = h0*2.04333385f + h1*-2.55058178f + h2*-2.57123921f + h3*-0.0163454061f + h4*2.49688221f + h5*1.3984721f + 0.224418875f;
  return z >= 0.0f ? 1 : 0;
}
};
}
