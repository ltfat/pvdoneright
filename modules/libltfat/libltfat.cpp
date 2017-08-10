#include "libltfat.h"

#ifdef DEBUG
#undef DEBUG
#endif

#ifdef LTFAT_SINGLE
#undef LTFAT_SINGLE
#endif

#ifdef LTFAT_DOUBLE
#undef LTFAT_DOUBLE
#endif

#ifdef LTFAT_COMPLEXTYPE
#undef LTFAT_COMPLEXTYPE
#endif

#define LTFAT_SINGLE
#include "libltfat/src/rtdgtreal.c"
#include "libltfat/src/ci_utils.c"
#include "libltfat/src/utils.c"
#if USEFFTW
#include "libltfat/src/fftw_wrappers.c"
#else
#include "libltfat/thirdparty/kissfft/fft.c"
#include "libltfat/src/kissfft_wrappers.c"
#endif
#include "libltfat/src/ci_memalloc.c"
#include "libltfat/src/heap.c"
#include "libltfat/src/gabdual_painless.c"
#include "libltfat/src/ci_windows.c"
#include "libpv/pv.c"
#include "libpv/rtpghi.c"

#include "libltfat/src/memalloc.c"
#define LTFAT_COMPLEXTYPE
#include "libltfat/src/ci_memalloc.c"
#undef LTFAT_COMPLEXTYPE

#undef LTFAT_SINGLE


#include "libltfat/src/error.c"
#include "libltfat/src/integer_manip.c"

