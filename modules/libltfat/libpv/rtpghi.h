#ifndef NOSYSTEMHEADERS
#include "ltfat.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ltfat/types.h"
// #include "phaseret/types.h"


/** Plan for rtpghi
 *
 * Serves for storing state between calls to rtpghi_execute.
 *
 */
typedef struct LTFAT_NAME(rtpghi_state) LTFAT_NAME(rtpghi_state);
typedef struct LTFAT_NAME(rtpghiupdate_plan) LTFAT_NAME(rtpghiupdate_plan);

/** \addtogroup rtpghi
 *  @{
 */

/** Create a RTPGHI state.
 *
 * \param[in]     gamma        Window-specific constant Cg*gl^2
 * \param[in]     W            Number of channels
 * \param[in]     a            Hop size
 * \param[in]     M            Number of frequency channels (FFT length)
 * \param[in]     tol          Relative coefficient tolerance.
 * \param[in]     do_causal    Zero delay (1) or 1 frame delay (0) version of the alg.
 * \param[out]    p            RTPGHI state
 *
 * #### Versions #
 * <tt>
 * phaseret_rtpghi_init_d(double gamma, ltfat_int W, ltfat_int a, ltfat_int M,
 *                        double tol, int do_causal, phaseret_rtpghi_state_d** p);
 *
 * phaseret_rtpghi_init_s(double gamma, ltfat_int W, ltfat_int a, ltfat_int M,
 *                        double tol, int do_causal, phaseret_rtpghi_state_s** p);
 * </tt>
 * \returns
 *
 * \see phaseret_firwin2gamma
 */
LTFAT_API int
LTFAT_NAME(rtpghi_init)(ltfat_int W, ltfat_int a, ltfat_int M,
                        double tol,
                        LTFAT_NAME(rtpghi_state)** p);

/** Reset RTPGHI state.
 *
 * Resets RTPGHI state struct to the initial state
 *
 * \param[out]    p            RTPGHI state
 *
 * #### Versions #
 * <tt>
 * phaseret_rtpghi_reset_d(phaseret_rtpghi_state_d* p);
 *
 * phaseret_rtpghi_reset_s(phaseret_rtpghi_state_s* p);
 * </tt>
 * \returns
 *
 */
LTFAT_API int
LTFAT_NAME(rtpghi_reset)(LTFAT_NAME(rtpghi_state)* p, const LTFAT_REAL** sinit );

/** Change tolerance
 *
 * \note This is not thread safe
 *
 * \param[in] p     RTPGHI plan
 * \param[in] tol   Relative tolerance
 *
 * #### Versions #
 * <tt>
 * phaseret_rtpghi_set_tol_d(phaseret_rtpghi_state_d* p, double tol);
 *
 * phaseret_rtpghi_set_tol_s(phaseret_rtpghi_state_s* p, double tol);
 * </tt>
 * \returns Status code
 */
LTFAT_API int
LTFAT_NAME(rtpghi_set_tol)(LTFAT_NAME(rtpghi_state)* p, double tol);

LTFAT_API double
LTFAT_NAME(rtpghi_get_stretch)(LTFAT_NAME(rtpghi_state)* p);

/** Execute RTPGHI plan for a single frame
 *
 *  The function is intedned to be called for consecutive stream of frames
 *  as it reuses some data from the previous frames stored in the plan.
 *
 *  if do_causal is enebled, c is not lagging, else c is lagging by one
 *  frame.
 *
 * \param[in]       p   RTPGHI plan
 * \param[in]       s   Target magnitude
 * \param[out]      c   Reconstructed coefficients
 *
 * #### Versions #
 * <tt>
 * phaseret_rtpghi_execute_d(phaseret_rtpghi_state_d* p, const double s[],
 *                           ltfat_complex_d c[]);
 *
 * phaseret_rtpghi_execute_s(phaseret_rtpghi_state_s* p, const float s[],
 *                           ltfat_complex_s c[]);
 * </tt>
 */
LTFAT_API int
LTFAT_NAME(rtpghi_execute)(LTFAT_NAME(rtpghi_state)* p,
                           const LTFAT_COMPLEX s[], double stretch, LTFAT_COMPLEX c[]);

/** Destroy a RTPGHI Plan.
 * \param[in] p  RTPGHI Plan
 *
 * #### Versions #
 * <tt>
 * phaseret_rtpghi_done_d(phaseret_rtpghi_state_d** p);
 *
 * phaseret_rtpghi_done_s(phaseret_rtpghi_state_s** p);
 * </tt>
 */
LTFAT_API int
LTFAT_NAME(rtpghi_done)(LTFAT_NAME(rtpghi_state)** p);

/** Do RTPGHI for a complete magnitude spectrogram and compensate delay
 *
 * This function just creates a plan, executes it for each col in s and c
 * and destroys it.
 *
 * \param[in]     s          Magnitude spectrogram  M2 x N array
 * \param[in]     gamma      Window-specific constant Cg*gl^2
 * \param[in]     L          Transform length (possibly zero-padded).
 * \param[in]     W          Number of signal channels.
 * \param[in]     a          Hop size
 * \param[in]     M          FFT length, also length of all the windows
 * \param[out]    c          Reconstructed coefficients M2 x N array
 *
 * #### Versions #
 * <tt>
 * phaseret_rtpghioffline_d(const double s[], double gamma,
 *                          ltfat_int L, ltfat_int W, ltfat_int a, ltfat_int M,
 *                          double tol, int do_causal, ltfat_complex_d c[]);
 *
 * phaseret_rtpghioffline_s(const float s[], double gamma,
 *                          ltfat_int L, ltfat_int W, ltfat_int a, ltfat_int M,
 *                          double tol, int do_causal, ltfat_complex_s c[]);
 * </tt>
 *
 * \see phaseret_firwin2gamma ltfat_dgtreal_phaseunlock
 */
LTFAT_API int
LTFAT_NAME(rtpghioffline)(const LTFAT_REAL s[],
                          ltfat_int L, ltfat_int W, ltfat_int a, ltfat_int M,
                          double gamma, double tol, int do_causal,
                          LTFAT_COMPLEX c[]);

/** @}*/

/** Compute phase frequency gradient by differentiation in time
 *
 * \param[in]     logs       Log-magnitude of a 3 x M2 buffer
 * \param[in]     a          Hop size
 * \param[in]     M          FFT length, also length of all the windows
 * \param[in]     gamma      Window-specific constant Cg*gl^2
 * \param[in]     do_causal  If true, fgrad is relevant for 3rd buffer col, else it
 *                           is relevant for 2nd buffer.
 * \param[out]    fgrad      Frequency gradient, array of length M2
 */
void
LTFAT_NAME(rtpghifgrad)(const LTFAT_REAL* phase, ltfat_int M,
                        double stretch, LTFAT_REAL* fgrad);

/** Compute phase time gradient by differentiation in frequency
 *
 * \param[in]     logs       Log-magnitude, array of length M2
 * \param[in]     a          Hop size
 * \param[in]     M          FFT length, also length of all the windows
 * \param[in]     gamma      Window-specific constant Cg*gl^2
 * \param[out]    tgrad      Time gradient, array of length M2
 */
void
LTFAT_NAME(rtpghitgrad)(const LTFAT_REAL* phase,
                        ltfat_int aanaprev, ltfat_int aananext, ltfat_int M,
                        double stretch, LTFAT_REAL* tgrad);


/** Combine magnitude and phase to a complex array
 * \param[in]        s      Magnitude, array of length L
 * \param[in]    phase      Phase in rad, array of length L
 * \param[in]        L      Length of the arrays
 * \param[out]       c      Output array of length L
 */
void
LTFAT_NAME(rtpghimagphase)(const LTFAT_REAL s[], const LTFAT_REAL phase[], ltfat_int L, LTFAT_COMPLEX c[]);

LTFAT_API int
LTFAT_NAME(rtpghiupdate_init)(ltfat_int M, ltfat_int W, double tol,
                              LTFAT_NAME(rtpghiupdate_plan)** pout);

LTFAT_API int
LTFAT_NAME(rtpghiupdate_execute)(LTFAT_NAME(rtpghiupdate_plan)* p,
                                 const LTFAT_REAL slog[],
                                 const LTFAT_REAL tgrad[],
                                 const LTFAT_REAL fgrad[],
                                 const LTFAT_REAL startphase[],
                                 LTFAT_REAL phase[]);

LTFAT_API int
LTFAT_NAME(rtpghiupdate_execute_withmask)(LTFAT_NAME(rtpghiupdate_plan)* p,
        const LTFAT_REAL slog[],
        const LTFAT_REAL tgrad[],
        const LTFAT_REAL fgrad[],
        const LTFAT_REAL startphase[],
        const int mask[], LTFAT_REAL phase[]);

LTFAT_API int
LTFAT_NAME(rtpghiupdate_execute_common)(LTFAT_NAME(rtpghiupdate_plan)* p,
                                        const LTFAT_REAL slog[],
                                        const LTFAT_REAL tgrad[],
                                        const LTFAT_REAL fgrad[],
                                        const LTFAT_REAL startphase[],
                                        LTFAT_REAL phase[]);

LTFAT_API int
LTFAT_NAME(rtpghiupdate_done)(LTFAT_NAME(rtpghiupdate_plan)** p);




#ifdef __cplusplus
}
#endif
