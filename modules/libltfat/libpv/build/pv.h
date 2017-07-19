#ifndef _RTPV_H
#define _RTPV_H
#include "ltfat.h"
typedef fftw_complex ltfat_complex_d;
typedef fftwf_complex ltfat_complex_s;
typedef struct ltfat_pv_state_s ltfat_pv_state_s;
 void
print_pos(ltfat_pv_state_s* state);
 void
ltfat_pv_advanceby_s(ltfat_pv_state_s* state, size_t Lin, size_t Lout);
 size_t
ltfat_pv_nextinlen_s(ltfat_pv_state_s* state, size_t Lout);
 size_t
ltfat_pv_nextoutlen_s(ltfat_pv_state_s* state, size_t Lin);
 int
ltfat_pv_init_s(double stretchmax, ltfat_int Wmax, ltfat_int bufLenMax,
                      ltfat_pv_state_s** pout);
 int
ltfat_pv_execute_s(ltfat_pv_state_s* p, const float* in[],
                         ltfat_int inLen, ltfat_int chanNo, double stretch,
                         ltfat_int outLen, float* out[]);
 int
ltfat_pv_execute_compact_s(ltfat_pv_state_s* p, const float* in,
                         ltfat_int inLen, ltfat_int chanNo, double stretch,
                         ltfat_int outLen, float* out);
 int
ltfat_pv_done_s(ltfat_pv_state_s** p);
 void
ltfat_pv_advanceby_s(ltfat_pv_state_s* state, size_t Lin,
                           size_t Lout);
 int
ltfat_pv_setstretch_s(ltfat_pv_state_s* state, double stretch);
#endif
