#ifndef _pv_h
#define _pv_h

#ifndef NOSYSTEMHEADERS
#include "ltfat.h"
#endif
#include "ltfat/types.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct LTFAT_NAME(pv_state) LTFAT_NAME(pv_state);


LTFAT_API void
print_pos(LTFAT_NAME(pv_state)* state);

LTFAT_API void
LTFAT_NAME(pv_advanceby)(LTFAT_NAME(pv_state)* state, size_t Lin, size_t Lout);

LTFAT_API size_t
LTFAT_NAME(pv_nextinlen)(LTFAT_NAME(pv_state)* state, size_t Lout);

LTFAT_API size_t
LTFAT_NAME(pv_nextoutlen)(LTFAT_NAME(pv_state)* state, size_t Lin);

LTFAT_API int
LTFAT_NAME(pv_init)(double stretchmax, ltfat_int Wmax, ltfat_int bufLenMax,
                      LTFAT_NAME(pv_state)** pout);

LTFAT_API int
LTFAT_NAME(pv_execute)(LTFAT_NAME(pv_state)* p, const LTFAT_REAL* in[],
                         ltfat_int inLen, ltfat_int chanNo, double stretch,
                         ltfat_int outLen, LTFAT_REAL* out[]);

LTFAT_API int
LTFAT_NAME(pv_execute_compact)(LTFAT_NAME(pv_state)* p, const LTFAT_REAL* in,
                         ltfat_int inLen, ltfat_int chanNo, double stretch,
                         ltfat_int outLen, LTFAT_REAL* out);


LTFAT_API int
LTFAT_NAME(pv_done)(LTFAT_NAME(pv_state)** p);

LTFAT_API void
LTFAT_NAME(pv_advanceby)(LTFAT_NAME(pv_state)* state, size_t Lin,
                           size_t Lout);

LTFAT_API int
LTFAT_NAME(pv_setstretch)(LTFAT_NAME(pv_state)* state, double stretch);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _pv_h */
