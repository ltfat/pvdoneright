#include "ltfat.h"
#include "ltfat/types.h"
#include "ltfat/macros.h"
#include "pv.h"
#include "rtpghi.h"

struct LTFAT_NAME(pv_state)
{
    LTFAT_NAME(rtdgtreal_processor_state)* procstate;
    LTFAT_NAME(rtpghi_state)* rtpghistate;
    double stretch;
    size_t in_pos;
    size_t out_pos;
    double in_in_out_offset;
    double out_in_in_offset;
    ltfat_int aana;
    ltfat_int asyn;
};

LTFAT_API void
print_pos(LTFAT_NAME(pv_state)* state)
{
    printf("in_pos: %zu, out_pos: %zu, in_out_offset: %.3f, out_in_offset: %.3f, stretch: %.3f\n",
           state->in_pos, state->out_pos, state->in_in_out_offset,
           state->out_in_in_offset, state->stretch);
}

LTFAT_API size_t
LTFAT_NAME(pv_nextinlen)(LTFAT_NAME(pv_state)* state, size_t Lout)
{
    double stretch = LTFAT_NAME(rtpghi_get_stretch)(state->rtpghistate);

    size_t in_pos_end = (size_t) round(Lout / stretch + state->out_in_in_offset);

    return in_pos_end;
}

LTFAT_API size_t
LTFAT_NAME(pv_nextoutlen)(LTFAT_NAME(pv_state)* state, size_t Lin)
{
    double stretch = LTFAT_NAME(rtpghi_get_stretch)(state->rtpghistate);

    size_t out_pos_end = (size_t) round(Lin * stretch + state->in_in_out_offset);

    return out_pos_end;
}


LTFAT_API void
LTFAT_NAME(pv_advanceby)(LTFAT_NAME(pv_state)* state, size_t Lin,
                         size_t Lout)
{
    //double stretch = state->stretch;
    double stretch = LTFAT_NAME(rtpghi_get_stretch)(state->rtpghistate);

    state->in_pos += Lin;
    state->out_pos += Lout;

    state->in_in_out_offset += Lin * stretch;
    state->in_in_out_offset -= Lout;

    state->out_in_in_offset += Lout / stretch;
    state->out_in_in_offset -= Lin;
}

LTFAT_API int
LTFAT_NAME(pv_setstretch)(LTFAT_NAME(pv_state)* state, double stretch)
{

    ltfat_int newaana = ltfat_round(state->asyn / stretch);
    double truestretch = ((double)state->asyn) / newaana;

    if (truestretch != state->stretch)
    {
        state->aana = newaana;
        state->stretch = truestretch;
        LTFAT_NAME(rtdgtreal_processor_setanaa)(state->procstate, newaana);
    }
    return 0;
}

static void
LTFAT_NAME(rtpghi_processor_callback)(void* userdata,
                                      const LTFAT_COMPLEX* in,
                                      int UNUSED(M2),
                                      int UNUSED(W),
                                      LTFAT_COMPLEX* out)
{
    LTFAT_NAME(pv_state)* state = (LTFAT_NAME(pv_state)*) userdata;
    LTFAT_NAME(rtpghi_execute)(state->rtpghistate, in, state->stretch, out);
}


LTFAT_API int
LTFAT_NAME(pv_init)(double stretchmax, ltfat_int Wmax, ltfat_int bufLenMax,
                    LTFAT_NAME(pv_state)** pout)
{
    int status = LTFATERR_FAILED;
    LTFAT_NAME(pv_state)* p = NULL;
    ltfat_int asyn  = 1024;
    ltfat_int M     = 8192;
    ltfat_int gl    = 4096;
    ltfat_int fifoSize = (bufLenMax + asyn) * stretchmax;
    ltfat_int procDelay = gl < fifoSize ? fifoSize : gl;

    CHECKMEM( p = (LTFAT_NAME(pv_state)*) ltfat_calloc(1, sizeof * p));
    p->asyn = asyn;

    CHECKSTATUS(
        LTFAT_NAME(rtdgtreal_processor_init_win)(
            LTFAT_HANN, gl, asyn, M, Wmax,
            fifoSize, gl,
            &p->procstate),
        "Processor initialization failed"
    );

    CHECKSTATUS(
        LTFAT_NAME(rtpghi_init)(Wmax, asyn, M, 1e-6, &p->rtpghistate),
        "RTPGHI initialization failed"
    );

    LTFAT_NAME(rtdgtreal_processor_setcallback)(
        p->procstate, LTFAT_NAME(rtpghi_processor_callback), p);

    LTFAT_NAME(pv_setstretch)(p, 1);

    *pout = p;
    return LTFATERR_SUCCESS;
error:
    if (p) LTFAT_NAME(pv_done)(&p);
    return status;
}

LTFAT_API int
LTFAT_NAME(pv_execute)(LTFAT_NAME(pv_state)* p, const LTFAT_REAL* in[],
                       ltfat_int inLen, ltfat_int chanNo, double stretch,
                       ltfat_int outLen, LTFAT_REAL* out[])
{
    int status;

    LTFAT_NAME(pv_advanceby)(p, inLen, outLen);

    LTFAT_NAME(pv_setstretch)(p, stretch);

    status =
        LTFAT_NAME(rtdgtreal_processor_execute_gen)(
            p->procstate, in, inLen, chanNo, outLen, out);

    return status;
}

LTFAT_API int
LTFAT_NAME(pv_execute_compact)(LTFAT_NAME(pv_state)* p, const LTFAT_REAL* in,
                               ltfat_int inLen, ltfat_int chanNo, double stretch,
                               ltfat_int outLen, LTFAT_REAL* out)
{
    int status;

    LTFAT_NAME(pv_advanceby)(p, inLen, outLen);

    LTFAT_NAME(pv_setstretch)(p, stretch);

    status =
        LTFAT_NAME(rtdgtreal_processor_execute_gen_compact)(
            p->procstate, in, inLen, chanNo, outLen, out);

    return status;
}


LTFAT_API int
LTFAT_NAME(pv_done)(LTFAT_NAME(pv_state)** p)
{
    LTFAT_NAME(pv_state)* pp;
    int status = LTFATERR_FAILED;
    CHECKNULL(p); CHECKNULL(*p);
    pp = *p;

    if (pp->procstate) LTFAT_NAME(rtdgtreal_processor_done)(&pp->procstate);
    if (pp->rtpghistate) LTFAT_NAME(rtpghi_done)(&pp->rtpghistate);
    ltfat_free(pp);

    pp = NULL;
    return LTFATERR_SUCCESS;
error:
    return status;

}
