#include "ltfat.h"
#include "ltfat/macros.h"
#include "ltfat/types.h"
#include "rtpghi.h"

struct LTFAT_NAME(rtpghi_state)
{
    LTFAT_NAME(rtpghiupdate_plan)* p;
    ltfat_int M;
    ltfat_int a;
    ltfat_int W;
    LTFAT_REAL* s;
    LTFAT_REAL* tgrad; //!< Time gradient buffer
    LTFAT_REAL* fgrad; //!< Frequency gradient buffer
    LTFAT_REAL* phase;
    LTFAT_REAL* phasein;
    double stretch;
};

struct LTFAT_NAME(rtpghiupdate_plan)
{
    LTFAT_NAME(heap)* h;
    int* donemask;
    double tol;
    ltfat_int M;
    LTFAT_REAL* randphase; //!< Precomputed array of random phase
    ltfat_int randphaseLen;
    ltfat_int randphaseId;
};




static LTFAT_REAL
LTFAT_NAME(princarg)(const LTFAT_REAL in)
{
    return (LTFAT_REAL) ( in - 2.0 * M_PI * round(in / (2.0 * M_PI)) );
}

static int
LTFAT_NAME(shiftcolsleft)(LTFAT_REAL* cols, ltfat_int height, ltfat_int N,
                          const LTFAT_REAL* newcol)
{
    for (ltfat_int n = 0; n < N - 1; n++)
        memcpy(cols + n * height, cols + (n + 1)*height, height * sizeof * cols);

    if (newcol)
        memcpy(cols + (N - 1)*height, newcol, height * sizeof * cols);
    else
        memset(cols + (N - 1)*height, 0, height * sizeof * cols);

    return 0;
}

static void
LTFAT_NAME(rtpghi_abs)(const LTFAT_COMPLEX in[], ltfat_int height,
                       LTFAT_REAL out[])
{
    for (ltfat_int ii = 0; ii < height; ii++)
        out[ii] = ltfat_abs(in[ii]);
}

static void
LTFAT_NAME(rtpghi_phase)(const LTFAT_COMPLEX in[], ltfat_int height,
                         LTFAT_REAL out[])
{
    for (ltfat_int ii = 0; ii < height; ii++)
        out[ii] = ltfat_arg(in[ii]);
}

LTFAT_API double
LTFAT_NAME(rtpghi_get_stretch)(LTFAT_NAME(rtpghi_state)* p)
{
    return p->stretch;
}


LTFAT_API int
LTFAT_NAME(rtpghi_set_tol)(LTFAT_NAME(rtpghi_state)* p, double tol)
{
    int status = LTFATERR_SUCCESS;
    CHECKNULL(p);
    CHECK(LTFATERR_NOTINRANGE, tol > 0 && tol < 1, "tol must be in range ]0,1[");

    p->p->tol = tol;
error:
    return status;
}

LTFAT_API int
LTFAT_NAME(rtpghi_init)(ltfat_int W, ltfat_int a, ltfat_int M,
                        double tol, LTFAT_NAME(rtpghi_state)** pout)
{
    int status = LTFATERR_SUCCESS;

    ltfat_int M2 = M / 2 + 1;
    LTFAT_NAME(rtpghi_state)* p = NULL;

    CHECK(LTFATERR_NOTPOSARG, W > 0, "W must be positive");
    CHECK(LTFATERR_NOTPOSARG, a > 0, "a must be positive (passed %d)", a);
    CHECK(LTFATERR_NOTPOSARG, M > 0, "M must be positive");
    CHECK(LTFATERR_NOTINRANGE, tol > 0 && tol < 1, "tol must be in range ]0,1[");

    CHECKMEM( p = (LTFAT_NAME(rtpghi_state)*) ltfat_calloc(1, sizeof * p));

    CHECKSTATUS( LTFAT_NAME(rtpghiupdate_init)( M, W, tol, &p->p),
                 "rtpghiupdate_init failed");
    CHECKMEM( p->s =       LTFAT_NAME_REAL(calloc)(3 * M2 * W));
    CHECKMEM( p->tgrad =   LTFAT_NAME_REAL(calloc)(2 * M2 * W));
    CHECKMEM( p->fgrad =   LTFAT_NAME_REAL(calloc)(1 * M2 * W));
    CHECKMEM( p->phase =   LTFAT_NAME_REAL(calloc)(1 * M2 * W));
    CHECKMEM( p->phasein = LTFAT_NAME_REAL(calloc)(3 * M2 * W));

    p->M = M;
    p->a = a;
    p->W = W;
    p->stretch = 1.0;

    *pout = p;
    return status;
error:
    if (p) LTFAT_NAME(rtpghi_done)(&p);
    return status;
}

LTFAT_API int
LTFAT_NAME(rtpghi_reset)(LTFAT_NAME(rtpghi_state)* p,
                         const LTFAT_REAL** sinit)
{
    int status = LTFATERR_SUCCESS;
    ltfat_int M2, W;
    CHECKNULL(p);
    M2 = p->M / 2 + 1;
    W = p->W;

    memset(p->s, 0,     3 * M2 * W * sizeof * p->s);
    memset(p->tgrad, 0, 2 * M2 * W * sizeof * p->tgrad);
    memset(p->fgrad, 0, 1 * M2 * W * sizeof * p->tgrad);
    memset(p->phase, 0, 1 * M2 * W * sizeof * p->phase);
    memset(p->phasein, 0, 3 * M2 * W * sizeof * p->phase);

    if (sinit)
        for (ltfat_int w = 0; w < W; w++)
            if (sinit[w])
            {
                memcpy(p->s + 2 * w * M2, sinit[w], 2 * M2 * sizeof * p->s );
            }

error:
    return status;
}

LTFAT_API int
LTFAT_NAME(rtpghi_execute)(LTFAT_NAME(rtpghi_state)* p,
                           const LTFAT_COMPLEX cin[], double stretch, LTFAT_COMPLEX cout[])
{
    // n, n-1, n-2 frames
    // s is n-th
    ltfat_int M2, W, asyn, aanaprev, aananext;
    double stretchmid;
    int status = LTFATERR_SUCCESS;
    CHECKNULL(p); CHECKNULL(cin); CHECKNULL(cout);
    M2 = p->M / 2 + 1; W = p->W;
    asyn = p->a;
    aanaprev = round(asyn / p->stretch); // old stretch
    aananext = round(asyn / stretch); // new stretch

    for (ltfat_int w = 0; w < W; ++w)
    {
        LTFAT_REAL* sCol       = p->s       + 3 * w * M2;
        LTFAT_REAL* tgradCol   = p->tgrad   + 2 * w * M2;
        LTFAT_REAL* fgradCol   = p->fgrad   + 1 * w * M2;
        LTFAT_REAL* phaseCol   = p->phase   + 1 * w * M2;
        LTFAT_REAL* phaseinCol = p->phasein + 3 * w * M2;

        LTFAT_NAME(shiftcolsleft)(sCol, M2, 3, NULL);
        LTFAT_NAME(shiftcolsleft)(tgradCol, M2, 2, NULL);
        LTFAT_NAME(shiftcolsleft)(phaseinCol, M2, 3, NULL);

        LTFAT_NAME(rtpghi_abs)(cin + w * M2,   M2, sCol + 2 * M2);

        LTFAT_NAME(rtpghi_phase)(cin + w * M2, M2, phaseinCol + 2 * M2);

        LTFAT_NAME(rtpghitgrad)(phaseinCol, aanaprev, aananext, p->M, p->stretch,
                                tgradCol + M2);

        if (ltfat_abs(stretch - 1.0) < 1e-4)
        {
            //Bypass if no stretching is done
            memcpy(phaseCol, phaseinCol + M2, M2 * sizeof * phaseCol);
        }
        else
        {
            LTFAT_NAME(rtpghifgrad)(phaseinCol + M2, p->M, p->stretch, fgradCol);

            LTFAT_NAME(rtpghiupdate_execute)(p->p,
                                             sCol, tgradCol, fgradCol, phaseCol, phaseCol);
        }
        // Combine phase with magnitude
        LTFAT_NAME(rtpghimagphase)( sCol + M2, phaseCol, M2, cout + w * M2);
    }

    // Only update stretch for the next frame
    p->stretch = stretch;
error:
    return status;
}

LTFAT_API int
LTFAT_NAME(rtpghi_done)(LTFAT_NAME(rtpghi_state)** p)
{
    int status = LTFATERR_SUCCESS;
    LTFAT_NAME(rtpghi_state)* pp;
    CHECKNULL(p); CHECKNULL(*p);
    pp = *p;
    if (pp->p)     LTFAT_NAME(rtpghiupdate_done)(&pp->p);
    if (pp->s)     ltfat_free(pp->s);
    if (pp->phase) ltfat_free(pp->phase);
    if (pp->tgrad) ltfat_free(pp->tgrad);
    if (pp->fgrad) ltfat_free(pp->fgrad);
    ltfat_free(pp);
    pp = NULL;
error:
    return status;
}

LTFAT_API int
LTFAT_NAME(rtpghiupdate_init)(ltfat_int M, ltfat_int W, double tol,
                              LTFAT_NAME(rtpghiupdate_plan)** pout)
{
    int status = LTFATERR_SUCCESS;

    ltfat_int M2 = M / 2 + 1;
    LTFAT_NAME(rtpghiupdate_plan)* p = NULL;
    CHECKMEM( p = (LTFAT_NAME(rtpghiupdate_plan)*) ltfat_calloc(1, sizeof * p));
    CHECKMEM( p->donemask = (int*) ltfat_calloc(M2, sizeof * p->donemask));

    p->randphaseLen = 10 * M2 * W;
    CHECKMEM( p->randphase = LTFAT_NAME_REAL(malloc)(p->randphaseLen));

    /* Do this somewhere else */
    for (ltfat_int ii = 0; ii < p->randphaseLen; ii++)
        p->randphase[ii] = 2.0 * M_PI * ((double)rand()) / RAND_MAX;

    p->tol = tol;
    p->M = M;
    p->randphaseId = 0;
    p->h = LTFAT_NAME(heap_init)(2 * M2, NULL);

    *pout = p;
    return status;
error:
    if (p) LTFAT_NAME(rtpghiupdate_done)(&p);
    return status;
}

LTFAT_API int
LTFAT_NAME(rtpghiupdate_execute_withmask)(LTFAT_NAME(
            rtpghiupdate_plan)* p,
        const LTFAT_REAL s[],
        const LTFAT_REAL tgrad[],
        const LTFAT_REAL fgrad[],
        const LTFAT_REAL startphase[],
        const int mask[], LTFAT_REAL phase[])
{
    ltfat_int M2 = p->M / 2 + 1;
    memcpy(p->donemask, mask, M2 * sizeof * p->donemask);

    return LTFAT_NAME(rtpghiupdate_execute_common)(p, s, tgrad, fgrad,
            startphase, phase);
}

// s: M2 x 2
// tgrad: M2 x 2
// fgrad: M2 x 1
// startphase: M2 x 1
// phase: M2 x 1
// donemask: M2 x 1
// heap must be able to hold 2*M2 values
LTFAT_API int
LTFAT_NAME(rtpghiupdate_execute)(LTFAT_NAME(rtpghiupdate_plan)* p,
                                 const LTFAT_REAL s[],
                                 const LTFAT_REAL tgrad[],
                                 const LTFAT_REAL fgrad[],
                                 const LTFAT_REAL startphase[],
                                 LTFAT_REAL phase[])
{
    ltfat_int M2 = p->M / 2 + 1;
    memset(p->donemask, 0, M2 * sizeof * p->donemask);

    return LTFAT_NAME(rtpghiupdate_execute_common)(p, s, tgrad, fgrad,
            startphase, phase);
}


LTFAT_API int
LTFAT_NAME(rtpghiupdate_execute_common)(LTFAT_NAME(rtpghiupdate_plan)* p,
                                        const LTFAT_REAL s[],
                                        const LTFAT_REAL tgrad[],
                                        const LTFAT_REAL fgrad[],
                                        const LTFAT_REAL startphase[],
                                        LTFAT_REAL phase[])
{
    LTFAT_NAME(heap)* h = p->h;
    ltfat_int M2 = p->M / 2 + 1;
    ltfat_int quickbreak = M2;
    const LTFAT_REAL oneover2 = (LTFAT_REAL) ( 1.0 / 2.0 );
    // We only need to compute M2 values, so perform quick exit
    // if we have them, but the heap is not yet empty.
    // (deleting hrom heap involves many operations)
    int* donemask = p->donemask;
    const LTFAT_REAL* slog2 = s + M2;

    // Find max and the absolute thrreshold
    LTFAT_REAL logabstol = s[0];
    for (ltfat_int m = 1; m < 2 * M2; m++)
        if (s[m] > logabstol)
            logabstol = s[m];

    logabstol *= p->tol;

    LTFAT_NAME(heap_reset)(h, s);

    for (ltfat_int m = 0; m < M2; m++)
    {
        if ( donemask[m] > 0 )
        {
            // We already know this one
            LTFAT_NAME(heap_insert)(h, m + M2);
            quickbreak--;
        }
        else
        {
            if ( slog2[m] <= logabstol )
            {
                // This will get a randomly generated phase
                donemask[m] = -1;
                quickbreak--;
            }
            else
            {
                LTFAT_NAME(heap_insert)(h, m);
            }
        }
    }

    ltfat_int w = -1;
    while ( (quickbreak > 0) && ( w = LTFAT_NAME(heap_delete)(h) ) >= 0 )
    {
        if ( w >= M2 )
        {
            // Next frame
            ltfat_int wprev = w - M2;

            if ( wprev != M2 - 1 && !donemask[wprev + 1] )
            {
                phase[wprev + 1] = phase[wprev] + (fgrad[wprev] + fgrad[wprev + 1]) * oneover2;
                donemask[wprev + 1] = 1;

                LTFAT_NAME(heap_insert)(h, w + 1);
                quickbreak--;
            }

            if ( wprev != 0 && !donemask[wprev - 1] )
            {
                phase[wprev - 1] = phase[wprev] - (fgrad[wprev] + fgrad[wprev - 1]) * oneover2;
                donemask[wprev - 1] = 1;

                LTFAT_NAME(heap_insert)(h, w - 1);
                quickbreak--;
            }
        }
        else
        {
            // Current frame
            if ( !donemask[w] )
            {
                ltfat_int wnext = w + M2;
                phase[w] = startphase[w] + (tgrad[w] + tgrad[wnext]) * oneover2;
                donemask[w] = 1;

                LTFAT_NAME(heap_insert)(h, wnext);
                quickbreak--;
            }
        }
    }

    // Fill in values below tol
    for (ltfat_int ii = 0; ii < M2; ii++)
    {
        if (donemask[ii] < 0)
        {
            phase[ii] = p->randphase[p->randphaseId++];
            p->randphaseId %= p->randphaseLen;
        }
    }

    return 0;
}

LTFAT_API int
LTFAT_NAME(rtpghiupdate_done)(LTFAT_NAME(rtpghiupdate_plan)** p)
{
    int status = LTFATERR_SUCCESS;
    LTFAT_NAME(rtpghiupdate_plan)* pp;
    CHECKNULL(p); CHECKNULL(*p);
    pp = *p;
    if (pp->h)         LTFAT_NAME(heap_done)(pp->h);
    if (pp->donemask)  ltfat_free(pp->donemask);
    if (pp->randphase) ltfat_free(pp->randphase);
    ltfat_free(pp);
    pp = NULL;
error:
    return status;
}


void
LTFAT_NAME(rtpghifgrad)(const LTFAT_REAL* phase, ltfat_int M, double stretch,
                        LTFAT_REAL* fgrad)
{
    ltfat_int M2 = M / 2 + 1;

    for (ltfat_int m = 1; m < M2 - 1; m++)
        fgrad[m] = ( LTFAT_NAME(princarg)(phase[m + 1] - phase[m]) +
                     LTFAT_NAME(princarg)(phase[m]     - phase[m - 1]) )
                   / (2.0) * stretch;

    fgrad[0]      = phase[0]*stretch;
    fgrad[M2 - 1] = phase[M2 - 1]*stretch;
}

void
LTFAT_NAME(rtpghitgrad)(const LTFAT_REAL* phase,
                        ltfat_int aanaprev, ltfat_int aananext, ltfat_int M,
                        double stretch, LTFAT_REAL* tgrad)
{
    ltfat_int M2 = M / 2 + 1;
    // a is asyn
    double asyn = aanaprev * stretch;

    LTFAT_REAL const1prev = 2.0 * M_PI * ((double) aanaprev) / M;
    LTFAT_REAL const1next = 2.0 * M_PI * ((double) aananext) / M;
    LTFAT_REAL const2 = 2.0 * M_PI * asyn / M;

    const LTFAT_REAL* pcol0 = phase;
    const LTFAT_REAL* pcol1 = phase + 1 * M2;
    const LTFAT_REAL* pcol2 = phase + 2 * M2;

    for (ltfat_int m = 0; m < M2; ++m)
        tgrad[m] = asyn * (
                       LTFAT_NAME(princarg)(pcol2[m] - pcol1[m] - const1next * m) / (2.0 * aananext) -
                       LTFAT_NAME(princarg)(pcol1[m] - pcol0[m] - const1prev * m) / (2.0 * aanaprev)
                   )
                   + const2 * m;
}

void
LTFAT_NAME(rtpghimagphase)(const LTFAT_REAL* s, const LTFAT_REAL* phase,
                           ltfat_int L, LTFAT_COMPLEX* c)
{
    for (ltfat_int l = 0; l < L; l++)
        c[l] = s[l] * (cos(phase[l]) + I * sin(phase[l]));
}

/* LTFAT_API int */
/* LTFAT_NAME(rtpghioffline)(const LTFAT_REAL* s, ltfat_int L, */
/*                              ltfat_int W, ltfat_int a, ltfat_int M, */
/*                              double gamma, double tol, int do_causal, */
/*                              LTFAT_COMPLEX* c) */
/* { */
/*     ltfat_int N = L / a; */
/*     ltfat_int M2 = M / 2 + 1; */
/*     LTFAT_NAME(rtpghi_state)* p = NULL; */
/*     int status = LTFATERR_SUCCESS; */
/*     CHECKNULL(s); CHECKNULL(c); */
/*  */
/*     CHECKSTATUS( LTFAT_NAME(rtpghi_init)(1, a, M, gamma, tol, do_causal, &p), */
/*                  "rtpghi init failed"); */
/*  */
/*     if (do_causal) */
/*     { */
/*         for (ltfat_int w = 0; w < W; w++) */
/*         { */
/*  */
/*             const LTFAT_REAL* schan = s + w * N * M2; */
/*             LTFAT_NAME(rtpghi_reset)(p, &schan); */
/*  */
/*             for (ltfat_int n = 0; n < N; ++n) */
/*             { */
/*                 const LTFAT_REAL* sncol = schan + n * M2; */
/*                 LTFAT_COMPLEX* cncol =    c + n * M2 + w * N * M2; */
/*                 LTFAT_NAME(rtpghi_execute)(p, sncol, cncol); */
/*             } */
/*         } */
/*     } */
/*     else */
/*     { */
/*         for (ltfat_int w = 0; w < W; w++) */
/*         { */
/*             const LTFAT_REAL* schan = s + w * N * M2; */
/*             LTFAT_NAME(rtpghi_reset)(p, &schan); */
/*  */
/*             for (ltfat_int n = 0, nahead = 1; nahead < N; ++n, ++nahead) */
/*             { */
/*                 const LTFAT_REAL* sncol = schan + nahead * M2; */
/*                 LTFAT_COMPLEX* cncol =    c + n * M2 + w * N * M2; */
/*                 LTFAT_NAME(rtpghi_execute)(p, sncol, cncol); */
/*             } */
/*  */
/*             LTFAT_NAME(rtpghi_execute)(p, s + w * N * M2, c + (N - 1) * M2 + w * N * M2); */
/*         } */
/*     } */
/*  */
/*     LTFAT_NAME(rtpghi_done)(&p); */
/* error: */
/*     return status; */
/* } */
