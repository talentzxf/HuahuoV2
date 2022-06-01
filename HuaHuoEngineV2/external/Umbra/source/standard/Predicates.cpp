// Copyright (c) 2009-2014 Umbra Software Ltd.
// All rights reserved. www.umbrasoftware.com

#include <standard/Predicates.hpp>
#include <standard/Base.hpp>

/*****************************************************************************/
/*                                                                           */
/*  Routines for Arbitrary Precision Floating-point Arithmetic               */
/*  and Fast Robust Geometric Predicates                                     */
/*  (predicates.c)                                                           */
/*                                                                           */
/*  May 18, 1996                                                             */
/*                                                                           */
/*  Placed in the public domain by                                           */
/*  Jonathan Richard Shewchuk                                                */
/*  School of Computer Science                                               */
/*  Carnegie Mellon University                                               */
/*  5000 Forbes Avenue                                                       */
/*  Pittsburgh, Pennsylvania  15213-3891                                     */
/*  jrs@cs.cmu.edu                                                           */
/*                                                                           */
/*  This file contains C implementation of algorithms for exact addition     */
/*    and multiplication of floating-point numbers, and predicates for       */
/*    robustly performing the orientation and incircle tests used in         */
/*    computational geometry.  The algorithms and underlying theory are      */
/*    described in Jonathan Richard Shewchuk.  "Adaptive Precision Floating- */
/*    Point Arithmetic and Fast Robust Geometric Predicates."  Technical     */
/*    Report CMU-CS-96-140, School of Computer Science, Carnegie Mellon      */
/*    University, Pittsburgh, Pennsylvania, May 1996.  (Submitted to         */
/*    Discrete & Computational Geometry.)                                    */
/*                                                                           */
/*  This file, the paper listed above, and other information are available   */
/*    from the Web page http://www.cs.cmu.edu/~quake/robust.html .           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Using this code:                                                         */
/*                                                                           */
/*  First, read the short or long version of the paper (from the Web page    */
/*    above).                                                                */
/*                                                                           */
/*  Be sure to call exactinit() once, before calling any of the arithmetic   */
/*    functions or geometric predicates.  Also be sure to turn on the        */
/*    optimizer when compiling this file.                                    */
/*                                                                           */
/*                                                                           */
/*  Several geometric predicates are defined.  Their parameters are all      */
/*    points.  Each point is an array of two or three floating-point         */
/*    numbers.  The geometric predicates, described in the papers, are       */
/*                                                                           */
/*    orient2d(pa, pb, pc)                                                   */
/*    orient2dfast(pa, pb, pc)                                               */
/*    orient3d(pa, pb, pc, pd)                                               */
/*    orient3dfast(pa, pb, pc, pd)                                           */
/*    incircle(pa, pb, pc, pd)                                               */
/*    incirclefast(pa, pb, pc, pd)                                           */
/*    insphere(pa, pb, pc, pd, pe)                                           */
/*    inspherefast(pa, pb, pc, pd, pe)                                       */
/*                                                                           */
/*  Those with suffix "fast" are approximate, non-robust versions.  Those    */
/*    without the suffix are adaptive precision, robust versions.  There     */
/*    are also versions with the suffices "exact" and "slow", which are      */
/*    non-adaptive, exact arithmetic versions, which I use only for timings  */
/*    in my arithmetic papers.                                               */
/*                                                                           */
/*                                                                           */
/*  An expansion is represented by an array of floating-point numbers,       */
/*    sorted from smallest to largest magnitude (possibly with interspersed  */
/*    zeros).  The length of each expansion is stored as a separate integer, */
/*    and each arithmetic function returns an integer which is the length    */
/*    of the expansion it created.                                           */
/*                                                                           */
/*  Several arithmetic functions are defined.  Their parameters are          */
/*                                                                           */
/*    e, f           Input expansions                                        */
/*    elen, flen     Lengths of input expansions (must be >= 1)              */
/*    h              Output expansion                                        */
/*    b              Input scalar                                            */
/*                                                                           */
/*  The arithmetic functions are                                             */
/*                                                                           */
/*    grow_expansion(elen, e, b, h)                                          */
/*    grow_expansion_zeroelim(elen, e, b, h)                                 */
/*    expansion_sum(elen, e, flen, f, h)                                     */
/*    expansion_sum_zeroelim1(elen, e, flen, f, h)                           */
/*    expansion_sum_zeroelim2(elen, e, flen, f, h)                           */
/*    fast_expansion_sum(elen, e, flen, f, h)                                */
/*    fast_expansion_sum_zeroelim(elen, e, flen, f, h)                       */
/*    linear_expansion_sum(elen, e, flen, f, h)                              */
/*    linear_expansion_sum_zeroelim(elen, e, flen, f, h)                     */
/*    scale_expansion(elen, e, b, h)                                         */
/*    scale_expansion_zeroelim(elen, e, b, h)                                */
/*    compress(elen, e, h)                                                   */
/*                                                                           */
/*  All of these are described in the long version of the paper; some are    */
/*    described in the short version.  All return an integer that is the     */
/*    length of h.  Those with suffix _zeroelim perform zero elimination,    */
/*    and are recommended over their counterparts.  The procedure            */
/*    fast_expansion_sum_zeroelim() (or linear_expansion_sum_zeroelim() on   */
/*    processors that do not use the round-to-even tiebreaking rule) is      */
/*    recommended over expansion_sum_zeroelim().  Each procedure has a       */
/*    little note next to it (in the code below) that tells you whether or   */
/*    not the output expansion may be the same array as one of the input     */
/*    expansions.                                                            */
/*                                                                           */
/*                                                                           */
/*  If you look around below, you'll also find macros for a bunch of         */
/*    simple unrolled arithmetic operations, and procedures for printing     */
/*    expansions (commented out because they don't work with all C           */
/*    compilers) and for generating random floating-point numbers whose      */
/*    significand bits are all random.  Most of the macros have undocumented */
/*    requirements that certain of their parameters should not be the same   */
/*    variable; for safety, better to make sure all the parameters are       */
/*    distinct variables.  Feel free to send email to jrs@cs.cmu.edu if you  */
/*    have questions.                                                        */
/*                                                                           */
/*****************************************************************************/

#if defined (_MSC_VER)
#   pragma warning(disable:4514)    /* unreferenced inline function has been removed        */
#   pragma warning(disable:4244)    /* conversion from double to float (done on purpose!)   */
#endif

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cfloat>                   // for FLT_MAX

/* On some machines, the exact arithmetic routines might be defeated by the  */
/*   use of internal extended precision floating-point registers.  Sometimes */
/*   this problem can be fixed by defining certain values to be volatile,    */
/*   thus forcing them to be stored to memory and rounded off.  This isn't   */
/*   a great solution, though, as it slows the arithmetic down.              */
/*                                                                           */
/* To try this out, write "#define INEXACT volatile" below.  Normally,       */
/*   however, INEXACT should be defined to be nothing.  ("#define INEXACT".) */

#define INEXACT                          /* Nothing */

// #define INEXACT volatile

#define REAL double                      /* float or double */

/* Which of the following two methods of finding the absolute values is      */
/*   fastest is compiler-dependent.  A few compilers can inline and optimize */
/*   the fabs() call; but most will incur the overhead of a function call,   */
/*   which is disastrously slow.  A faster way on IEEE machines might be to  */
/*   mask the appropriate bit, but that's difficult to do in C.              */

#define Absolute(a)  (std::fabs(a)) /*//(a) >= 0.0 ? (a) : -(a))*/

/* Many of the operations are broken up into two pieces, a main part that    */
/*   performs an approximate operation, and a "tail" that computes the       */
/*   roundoff error of that operation.                                       */
/*                                                                           */
/* The operations Fast_Two_Sum(), Fast_Two_Diff(), Two_Sum(), Two_Diff(),    */
/*   Split(), and Two_Product() are all implemented as described in the      */
/*   reference.  Each of these macros requires certain variables to be       */
/*   defined in the calling routine.  The variables `bvirt', `c', `abig',    */
/*   `_i', `_j', `_k', `_l', `_m', and `_n' are declared `INEXACT' because   */
/*   they store the result of an operation that may incur roundoff error.    */
/*   The input parameter `x' (or the highest numbered `x_' parameter) must   */
/*   also be declared `INEXACT'.                                             */

#define Fast_Two_Sum_Tail(a, b, x, y) \
  bvirt = x - a; \
  y = b - bvirt

#define Fast_Two_Sum(a, b, x, y) \
  x = (REAL) (a + b); \
  Fast_Two_Sum_Tail(a, b, x, y)

#define Fast_Two_Diff_Tail(a, b, x, y) \
  bvirt = a - x; \
  y = bvirt - b

#define Fast_Two_Diff(a, b, x, y) \
  x = (REAL) (a - b); \
  Fast_Two_Diff_Tail(a, b, x, y)

#define Two_Sum_Tail(a, b, x, y) \
  bvirt = (REAL) (x - a); \
  avirt = x - bvirt; \
  bround = b - bvirt; \
  around = a - avirt; \
  y = around + bround

#define Two_Sum(a, b, x, y) \
  x = (REAL) (a + b); \
  Two_Sum_Tail(a, b, x, y)

#define Two_Diff_Tail(a, b, x, y) \
  bvirt = (REAL) (a - x); \
  avirt = x + bvirt; \
  bround = bvirt - b; \
  around = a - avirt; \
  y = around + bround

#define Two_Diff(a, b, x, y) \
  x = (REAL) (a - b); \
  Two_Diff_Tail(a, b, x, y)

#define Split(a, ahi, alo) \
  c = (REAL) (splitter * a); \
  abig = (REAL) (c - a); \
  ahi = c - abig; \
  alo = a - ahi

#define Two_Product_Tail(a, b, x, y) \
  Split(a, ahi, alo); \
  Split(b, bhi, blo); \
  err1 = x - (ahi * bhi); \
  err2 = err1 - (alo * bhi); \
  err3 = err2 - (ahi * blo); \
  y = (alo * blo) - err3

#define Two_Product(a, b, x, y) \
  x = (REAL) (a * b); \
  Two_Product_Tail(a, b, x, y)

/* Two_Product_Presplit() is Two_Product() where one of the inputs has       */
/*   already been split.  Avoids redundant splitting.                        */

#define Two_Product_Presplit(a, b, bhi, blo, x, y) \
  x = (REAL) (a * b); \
  Split(a, ahi, alo); \
  err1 = x - (ahi * bhi); \
  err2 = err1 - (alo * bhi); \
  err3 = err2 - (ahi * blo); \
  y = (alo * blo) - err3



/* Macros for summing expansions of various fixed lengths.  These are all    */
/*   unrolled versions of Expansion_Sum().                                   */

#define Two_One_Sum(a1, a0, b, x2, x1, x0) \
  Two_Sum(a0, b , _i, x0); \
  Two_Sum(a1, _i, x2, x1)

#define Two_One_Diff(a1, a0, b, x2, x1, x0) \
  Two_Diff(a0, b , _i, x0); \
  Two_Sum( a1, _i, x2, x1)

#define Two_Two_Sum(a1, a0, b1, b0, x3, x2, x1, x0) \
  Two_One_Sum(a1, a0, b0, _j, _0, x0); \
  Two_One_Sum(_j, _0, b1, x3, x2, x1)

#define Two_Two_Diff(a1, a0, b1, b0, x3, x2, x1, x0) \
  Two_One_Diff(a1, a0, b0, _j, _0, x0); \
  Two_One_Diff(_j, _0, b1, x3, x2, x1)

/* Macros for multiplying expansions of various fixed lengths.               */

#define Two_One_Product(a1, a0, b, x3, x2, x1, x0) \
  Split(b, bhi, blo); \
  Two_Product_Presplit(a0, b, bhi, blo, _i, x0); \
  Two_Product_Presplit(a1, b, bhi, blo, _j, _0); \
  Two_Sum(_i, _0, _k, x1); \
  Fast_Two_Sum(_j, _k, x3, x2)

namespace Umbra {

static bool g_predInitialized = false;

class Pred
{
private:

    REAL splitter;          /* = 2^ceiling(p / 2) + 1.  Used to split floats in half. */
    REAL epsilon;           /* = 2^(-p).  Used to estimate roundoff errors. */
    REAL resulterrbound;
    REAL ccwerrboundA, ccwerrboundB, ccwerrboundC;
    REAL o3derrboundA, o3derrboundB, o3derrboundC;


/*****************************************************************************/
/*                                                                           */
/*  fast_expansion_sum_zeroelim()   Sum two expansions, eliminating zero     */
/*                                  components from the output expansion.    */
/*                                                                           */
/*  Sets h = e + f.  See the long version of my paper for details.           */
/*                                                                           */
/*  If round-to-even is used (as with IEEE 754), maintains the strongly      */
/*  nonoverlapping property.  (That is, if e is strongly nonoverlapping, h   */
/*  will be also.)  Does NOT maintain the nonoverlapping or nonadjacent      */
/*  properties.                                                              */
/*                                                                           */
/*****************************************************************************/

int fast_expansion_sum_zeroelim(
    int         elen,
    const REAL* e,
    int         flen,
    const REAL* f,
    REAL*       h)  const /* h cannot be e or f. */
{
    REAL            Q;
    INEXACT REAL    Qnew;
    INEXACT REAL    hh;
    INEXACT REAL    bvirt;
    REAL            avirt, bround, around;
    int             eindex, findex, hindex;
    REAL            enow, fnow;

    enow = e[0];
    fnow = f[0];
    eindex = findex = 0;
    if ((fnow > enow) == (fnow > -enow)) {
      Q = enow;
      enow = e[++eindex];
    } else {
      Q = fnow;
      fnow = f[++findex];
    }
    hindex = 0;
    if ((eindex < elen) && (findex < flen)) {
      if ((fnow > enow) == (fnow > -enow)) {
        Fast_Two_Sum(enow, Q, Qnew, hh);
        enow = e[++eindex];
      } else {
        Fast_Two_Sum(fnow, Q, Qnew, hh);
        fnow = f[++findex];
      }
      Q = Qnew;
      if (hh != 0.0) {
        h[hindex++] = hh;
      }
      while ((eindex < elen) && (findex < flen)) {
        if ((fnow > enow) == (fnow > -enow)) {
          Two_Sum(Q, enow, Qnew, hh);
          enow = e[++eindex];
        } else {
          Two_Sum(Q, fnow, Qnew, hh);
          fnow = f[++findex];
        }
        Q = Qnew;
        if (hh != 0.0) {
          h[hindex++] = hh;
        }
      }
    }
    while (eindex < elen) {
      Two_Sum(Q, enow, Qnew, hh);
      enow = e[++eindex];
      Q = Qnew;
      if (hh != 0.0) {
        h[hindex++] = hh;
      }
    }
    while (findex < flen) {
      Two_Sum(Q, fnow, Qnew, hh);
      fnow = f[++findex];
      Q = Qnew;
      if (hh != 0.0) {
        h[hindex++] = hh;
      }
    }
    if ((Q != 0.0) || (hindex == 0)) {
      h[hindex++] = Q;
    }
    return hindex;
}

/*****************************************************************************/
/*                                                                           */
/*  scale_expansion_zeroelim()   Multiply an expansion by a scalar,          */
/*                               eliminating zero components from the        */
/*                               output expansion.                           */
/*                                                                           */
/*  Sets h = be.  See either version of my paper for details.                */
/*                                                                           */
/*  Maintains the nonoverlapping property.  If round-to-even is used (as     */
/*  with IEEE 754), maintains the strongly nonoverlapping and nonadjacent    */
/*  properties as well.  (That is, if e has one of these properties, so      */
/*  will h.)                                                                 */
/*                                                                           */
/*****************************************************************************/

int scale_expansion_zeroelim(
    int         elen,
    const REAL* e,
    REAL        b,
    REAL*       h) const   /* e and h cannot be the same. */
{
    INEXACT REAL    Q, sum;
    REAL            hh;
    INEXACT REAL    product1;
    REAL            product0;
    int             eindex, hindex;
    REAL            enow;
    INEXACT REAL    bvirt;
    REAL            avirt, bround, around;
    INEXACT REAL    c;
    INEXACT REAL    abig;
    REAL            ahi, alo, bhi, blo;
    REAL            err1, err2, err3;

    Split(b, bhi, blo);
    Two_Product_Presplit(e[0], b, bhi, blo, Q, hh);
    hindex = 0;
    if (hh != 0)
    {
        h[hindex++] = hh;
    }
    for (eindex = 1; eindex < elen; eindex++) {
        enow = e[eindex];
        Two_Product_Presplit(enow, b, bhi, blo, product1, product0);
        Two_Sum(Q, product0, sum, hh);
        if (hh != 0)
        {
            h[hindex++] = hh;
        }
        Fast_Two_Sum(product1, sum, Q, hh);
        if (hh != 0)
        {
            h[hindex++] = hh;
        }
    }

    if ((Q != 0.0) || (hindex == 0))
    {
        h[hindex++] = Q;
    }
    return hindex;
}

/*****************************************************************************/
/*                                                                           */
/*  estimate()   Produce a one-word estimate of an expansion's value.        */
/*                                                                           */
/*  See either version of my paper for details.                              */
/*                                                                           */
/*****************************************************************************/

static UMBRA_FORCE_INLINE REAL estimate (
    int         elen,
    const REAL* e)
{
    REAL    Q;
    int     eindex;

    Q = e[0];
    for (eindex = 1; eindex < elen; eindex++)
    {
        Q += e[eindex];
    }

    return Q;
}

static REAL orient2dfast(
    const double* pa,
    const double* pb,
    const double* pc)
{
    REAL acx, bcx, acy, bcy;
    acx = pa[0] - pc[0];
    bcx = pb[0] - pc[0];
    acy = pa[1] - pc[1];
    bcy = pb[1] - pc[1];
    return acx * bcy - acy * bcx;
}

UMBRA_FORCE_INLINE REAL orient2dadapt(
    const double* pa,
    const double* pb,
    const double* pc,
    REAL detsum) const
{
    INEXACT REAL acx, acy, bcx, bcy;
    REAL acxtail, acytail, bcxtail, bcytail;
    INEXACT REAL detleft, detright;
    REAL detlefttail, detrighttail;
    REAL det, errbound;
    INEXACT REAL B3;
    int C1length, C2length, Dlength;
    REAL u[4];
    INEXACT REAL u3;
    INEXACT REAL s1, t1;
    REAL s0, t0;

    INEXACT REAL bvirt;
    REAL avirt, bround, around;
    INEXACT REAL c;
    INEXACT REAL abig;
    REAL ahi, alo, bhi, blo;
    REAL err1, err2, err3;
    INEXACT REAL _i, _j;
    REAL _0;
    REAL B[4], C1[8], C2[12], D[16];

    acx = (REAL) (pa[0] - pc[0]);
    bcx = (REAL) (pb[0] - pc[0]);
    acy = (REAL) (pa[1] - pc[1]);
    bcy = (REAL) (pb[1] - pc[1]);

    Two_Product(acx, bcy, detleft, detlefttail);
    Two_Product(acy, bcx, detright, detrighttail);

    Two_Two_Diff(detleft, detlefttail, detright, detrighttail,
                 B3, B[2], B[1], B[0]);
    B[3] = B3;

    det = estimate(4, B);
    errbound = ccwerrboundB * detsum;
    if ((det >= errbound) || (-det >= errbound)) {
      return det;
    }

    Two_Diff_Tail(pa[0], pc[0], acx, acxtail);
    Two_Diff_Tail(pb[0], pc[0], bcx, bcxtail);
    Two_Diff_Tail(pa[1], pc[1], acy, acytail);
    Two_Diff_Tail(pb[1], pc[1], bcy, bcytail);

    if ((acxtail == 0.0) && (acytail == 0.0)
        && (bcxtail == 0.0) && (bcytail == 0.0)) {
      return det;
    }

    errbound = ccwerrboundC * detsum + resulterrbound * Absolute(det);
    det += (acx * bcytail + bcy * acxtail)
         - (acy * bcxtail + bcx * acytail);
    if ((det >= errbound) || (-det >= errbound)) {
      return det;
    }

    Two_Product(acxtail, bcy, s1, s0);
    Two_Product(acytail, bcx, t1, t0);
    Two_Two_Diff(s1, s0, t1, t0, u3, u[2], u[1], u[0]);
    u[3] = u3;
    C1length = fast_expansion_sum_zeroelim(4, B, 4, u, C1);

    Two_Product(acx, bcytail, s1, s0);
    Two_Product(acy, bcxtail, t1, t0);
    Two_Two_Diff(s1, s0, t1, t0, u3, u[2], u[1], u[0]);
    u[3] = u3;
    C2length = fast_expansion_sum_zeroelim(C1length, C1, 4, u, C2);

    Two_Product(acxtail, bcytail, s1, s0);
    Two_Product(acytail, bcxtail, t1, t0);
    Two_Two_Diff(s1, s0, t1, t0, u3, u[2], u[1], u[0]);
    u[3] = u3;
    Dlength = fast_expansion_sum_zeroelim(C2length, C2, 4, u, D);

    return(D[Dlength - 1]);
}

UMBRA_FORCE_INLINE REAL orient3dadapt (
    const double* pa,
    const double* pb,
    const double* pc,
    const double* pd,
    REAL permanent) const
{
    REAL u[4], v[12], w[16];
    REAL fin1[192], fin2[192];
    REAL at_b[4], at_c[4], bt_c[4], bt_a[4], ct_a[4], ct_b[4];
    REAL bct[8], cat[8], abt[8];
    INEXACT REAL adx, bdx, cdx, ady, bdy, cdy, adz, bdz, cdz;
    REAL det, errbound;

    INEXACT REAL bdxcdy1, cdxbdy1, cdxady1, adxcdy1, adxbdy1, bdxady1;
    REAL bdxcdy0, cdxbdy0, cdxady0, adxcdy0, adxbdy0, bdxady0;
    REAL bc[4], ca[4], ab[4];
    INEXACT REAL bc3, ca3, ab3;
    REAL adet[8], bdet[8], cdet[8];
    int alen, blen, clen;
    REAL abdet[16];
    int ablen;
    REAL *finnow, *finother, *finswap;
    int finlength;

    REAL adxtail, bdxtail, cdxtail;
    REAL adytail, bdytail, cdytail;
    REAL adztail, bdztail, cdztail;
    INEXACT REAL at_blarge, at_clarge;
    INEXACT REAL bt_clarge, bt_alarge;
    INEXACT REAL ct_alarge, ct_blarge;
    int at_blen, at_clen, bt_clen, bt_alen, ct_alen, ct_blen;
    INEXACT REAL bdxt_cdy1, cdxt_bdy1, cdxt_ady1;
    INEXACT REAL adxt_cdy1, adxt_bdy1, bdxt_ady1;
    REAL bdxt_cdy0, cdxt_bdy0, cdxt_ady0;
    REAL adxt_cdy0, adxt_bdy0, bdxt_ady0;
    INEXACT REAL bdyt_cdx1, cdyt_bdx1, cdyt_adx1;
    INEXACT REAL adyt_cdx1, adyt_bdx1, bdyt_adx1;
    REAL bdyt_cdx0, cdyt_bdx0, cdyt_adx0;
    REAL adyt_cdx0, adyt_bdx0, bdyt_adx0;
    int bctlen, catlen, abtlen;
    INEXACT REAL bdxt_cdyt1, cdxt_bdyt1, cdxt_adyt1;
    INEXACT REAL adxt_cdyt1, adxt_bdyt1, bdxt_adyt1;
    REAL bdxt_cdyt0, cdxt_bdyt0, cdxt_adyt0;
    REAL adxt_cdyt0, adxt_bdyt0, bdxt_adyt0;
    INEXACT REAL u3;
    int vlength, wlength;
    REAL negate;

    INEXACT REAL bvirt;
    REAL avirt, bround, around;
    INEXACT REAL c;
    INEXACT REAL abig;
    REAL ahi, alo, bhi, blo;
    REAL err1, err2, err3;
    INEXACT REAL _i, _j, _k;
    REAL _0;

    adx = (REAL) (pa[0] - pd[0]);
    bdx = (REAL) (pb[0] - pd[0]);
    cdx = (REAL) (pc[0] - pd[0]);
    ady = (REAL) (pa[1] - pd[1]);
    bdy = (REAL) (pb[1] - pd[1]);
    cdy = (REAL) (pc[1] - pd[1]);
    adz = (REAL) (pa[2] - pd[2]);
    bdz = (REAL) (pb[2] - pd[2]);
    cdz = (REAL) (pc[2] - pd[2]);

    Two_Product(bdx, cdy, bdxcdy1, bdxcdy0);
    Two_Product(cdx, bdy, cdxbdy1, cdxbdy0);
    Two_Two_Diff(bdxcdy1, bdxcdy0, cdxbdy1, cdxbdy0, bc3, bc[2], bc[1], bc[0]);
    bc[3] = bc3;
    alen = scale_expansion_zeroelim(4, bc, adz, adet);

    Two_Product(cdx, ady, cdxady1, cdxady0);
    Two_Product(adx, cdy, adxcdy1, adxcdy0);
    Two_Two_Diff(cdxady1, cdxady0, adxcdy1, adxcdy0, ca3, ca[2], ca[1], ca[0]);
    ca[3] = ca3;
    blen = scale_expansion_zeroelim(4, ca, bdz, bdet);

    Two_Product(adx, bdy, adxbdy1, adxbdy0);
    Two_Product(bdx, ady, bdxady1, bdxady0);
    Two_Two_Diff(adxbdy1, adxbdy0, bdxady1, bdxady0, ab3, ab[2], ab[1], ab[0]);
    ab[3] = ab3;
    clen = scale_expansion_zeroelim(4, ab, cdz, cdet);

    ablen = fast_expansion_sum_zeroelim(alen, adet, blen, bdet, abdet);
    finlength = fast_expansion_sum_zeroelim(ablen, abdet, clen, cdet, fin1);

    det = estimate(finlength, fin1);
    errbound = o3derrboundB * permanent;
    if ((det >= errbound) || (-det >= errbound)) {
      return det;
    }

    Two_Diff_Tail(pa[0], pd[0], adx, adxtail);
    Two_Diff_Tail(pb[0], pd[0], bdx, bdxtail);
    Two_Diff_Tail(pc[0], pd[0], cdx, cdxtail);
    Two_Diff_Tail(pa[1], pd[1], ady, adytail);
    Two_Diff_Tail(pb[1], pd[1], bdy, bdytail);
    Two_Diff_Tail(pc[1], pd[1], cdy, cdytail);
    Two_Diff_Tail(pa[2], pd[2], adz, adztail);
    Two_Diff_Tail(pb[2], pd[2], bdz, bdztail);
    Two_Diff_Tail(pc[2], pd[2], cdz, cdztail);

    if ((adxtail == 0.0) && (bdxtail == 0.0) && (cdxtail == 0.0)
        && (adytail == 0.0) && (bdytail == 0.0) && (cdytail == 0.0)
        && (adztail == 0.0) && (bdztail == 0.0) && (cdztail == 0.0)) {
      return det;
    }

    errbound = o3derrboundC * permanent + resulterrbound * Absolute(det);
    det += (adz * ((bdx * cdytail + cdy * bdxtail)
                   - (bdy * cdxtail + cdx * bdytail))
            + adztail * (bdx * cdy - bdy * cdx))
         + (bdz * ((cdx * adytail + ady * cdxtail)
                   - (cdy * adxtail + adx * cdytail))
            + bdztail * (cdx * ady - cdy * adx))
         + (cdz * ((adx * bdytail + bdy * adxtail)
                   - (ady * bdxtail + bdx * adytail))
            + cdztail * (adx * bdy - ady * bdx));
    if ((det >= errbound) || (-det >= errbound)) {
      return det;
    }

    finnow = fin1;
    finother = fin2;

    if (adxtail == 0.0) {
      if (adytail == 0.0) {
        at_b[0] = 0.0;
        at_blen = 1;
        at_c[0] = 0.0;
        at_clen = 1;
      } else {
        negate = -adytail;
        Two_Product(negate, bdx, at_blarge, at_b[0]);
        at_b[1] = at_blarge;
        at_blen = 2;
        Two_Product(adytail, cdx, at_clarge, at_c[0]);
        at_c[1] = at_clarge;
        at_clen = 2;
      }
    } else {
      if (adytail == 0.0) {
        Two_Product(adxtail, bdy, at_blarge, at_b[0]);
        at_b[1] = at_blarge;
        at_blen = 2;
        negate = -adxtail;
        Two_Product(negate, cdy, at_clarge, at_c[0]);
        at_c[1] = at_clarge;
        at_clen = 2;
      } else {
        Two_Product(adxtail, bdy, adxt_bdy1, adxt_bdy0);
        Two_Product(adytail, bdx, adyt_bdx1, adyt_bdx0);
        Two_Two_Diff(adxt_bdy1, adxt_bdy0, adyt_bdx1, adyt_bdx0,
                     at_blarge, at_b[2], at_b[1], at_b[0]);
        at_b[3] = at_blarge;
        at_blen = 4;
        Two_Product(adytail, cdx, adyt_cdx1, adyt_cdx0);
        Two_Product(adxtail, cdy, adxt_cdy1, adxt_cdy0);
        Two_Two_Diff(adyt_cdx1, adyt_cdx0, adxt_cdy1, adxt_cdy0,
                     at_clarge, at_c[2], at_c[1], at_c[0]);
        at_c[3] = at_clarge;
        at_clen = 4;
      }
    }
    if (bdxtail == 0.0) {
      if (bdytail == 0.0) {
        bt_c[0] = 0.0;
        bt_clen = 1;
        bt_a[0] = 0.0;
        bt_alen = 1;
      } else {
        negate = -bdytail;
        Two_Product(negate, cdx, bt_clarge, bt_c[0]);
        bt_c[1] = bt_clarge;
        bt_clen = 2;
        Two_Product(bdytail, adx, bt_alarge, bt_a[0]);
        bt_a[1] = bt_alarge;
        bt_alen = 2;
      }
    } else {
      if (bdytail == 0.0) {
        Two_Product(bdxtail, cdy, bt_clarge, bt_c[0]);
        bt_c[1] = bt_clarge;
        bt_clen = 2;
        negate = -bdxtail;
        Two_Product(negate, ady, bt_alarge, bt_a[0]);
        bt_a[1] = bt_alarge;
        bt_alen = 2;
      } else {
        Two_Product(bdxtail, cdy, bdxt_cdy1, bdxt_cdy0);
        Two_Product(bdytail, cdx, bdyt_cdx1, bdyt_cdx0);
        Two_Two_Diff(bdxt_cdy1, bdxt_cdy0, bdyt_cdx1, bdyt_cdx0,
                     bt_clarge, bt_c[2], bt_c[1], bt_c[0]);
        bt_c[3] = bt_clarge;
        bt_clen = 4;
        Two_Product(bdytail, adx, bdyt_adx1, bdyt_adx0);
        Two_Product(bdxtail, ady, bdxt_ady1, bdxt_ady0);
        Two_Two_Diff(bdyt_adx1, bdyt_adx0, bdxt_ady1, bdxt_ady0,
                    bt_alarge, bt_a[2], bt_a[1], bt_a[0]);
        bt_a[3] = bt_alarge;
        bt_alen = 4;
      }
    }
    if (cdxtail == 0.0) {
      if (cdytail == 0.0) {
        ct_a[0] = 0.0;
        ct_alen = 1;
        ct_b[0] = 0.0;
        ct_blen = 1;
      } else {
        negate = -cdytail;
        Two_Product(negate, adx, ct_alarge, ct_a[0]);
        ct_a[1] = ct_alarge;
        ct_alen = 2;
        Two_Product(cdytail, bdx, ct_blarge, ct_b[0]);
        ct_b[1] = ct_blarge;
        ct_blen = 2;
      }
    } else {
      if (cdytail == 0.0) {
        Two_Product(cdxtail, ady, ct_alarge, ct_a[0]);
        ct_a[1] = ct_alarge;
        ct_alen = 2;
        negate = -cdxtail;
        Two_Product(negate, bdy, ct_blarge, ct_b[0]);
        ct_b[1] = ct_blarge;
        ct_blen = 2;
      } else {
        Two_Product(cdxtail, ady, cdxt_ady1, cdxt_ady0);
        Two_Product(cdytail, adx, cdyt_adx1, cdyt_adx0);
        Two_Two_Diff(cdxt_ady1, cdxt_ady0, cdyt_adx1, cdyt_adx0,
                     ct_alarge, ct_a[2], ct_a[1], ct_a[0]);
        ct_a[3] = ct_alarge;
        ct_alen = 4;
        Two_Product(cdytail, bdx, cdyt_bdx1, cdyt_bdx0);
        Two_Product(cdxtail, bdy, cdxt_bdy1, cdxt_bdy0);
        Two_Two_Diff(cdyt_bdx1, cdyt_bdx0, cdxt_bdy1, cdxt_bdy0,
                     ct_blarge, ct_b[2], ct_b[1], ct_b[0]);
        ct_b[3] = ct_blarge;
        ct_blen = 4;
      }
    }

    bctlen = fast_expansion_sum_zeroelim(bt_clen, bt_c, ct_blen, ct_b, bct);
    wlength = scale_expansion_zeroelim(bctlen, bct, adz, w);
    finlength = fast_expansion_sum_zeroelim(finlength, finnow, wlength, w,
                                            finother);
    finswap = finnow; finnow = finother; finother = finswap;

    catlen = fast_expansion_sum_zeroelim(ct_alen, ct_a, at_clen, at_c, cat);
    wlength = scale_expansion_zeroelim(catlen, cat, bdz, w);
    finlength = fast_expansion_sum_zeroelim(finlength, finnow, wlength, w,
                                            finother);
    finswap = finnow; finnow = finother; finother = finswap;

    abtlen = fast_expansion_sum_zeroelim(at_blen, at_b, bt_alen, bt_a, abt);
    wlength = scale_expansion_zeroelim(abtlen, abt, cdz, w);
    finlength = fast_expansion_sum_zeroelim(finlength, finnow, wlength, w,
                                            finother);
    finswap = finnow; finnow = finother; finother = finswap;

    if (adztail != 0.0) {
      vlength = scale_expansion_zeroelim(4, bc, adztail, v);
      finlength = fast_expansion_sum_zeroelim(finlength, finnow, vlength, v,
                                              finother);
      finswap = finnow; finnow = finother; finother = finswap;
    }
    if (bdztail != 0.0) {
      vlength = scale_expansion_zeroelim(4, ca, bdztail, v);
      finlength = fast_expansion_sum_zeroelim(finlength, finnow, vlength, v,
                                              finother);
      finswap = finnow; finnow = finother; finother = finswap;
    }
    if (cdztail != 0.0) {
      vlength = scale_expansion_zeroelim(4, ab, cdztail, v);
      finlength = fast_expansion_sum_zeroelim(finlength, finnow, vlength, v,
                                              finother);
      finswap = finnow; finnow = finother; finother = finswap;
    }

    if (adxtail != 0.0) {
      if (bdytail != 0.0) {
        Two_Product(adxtail, bdytail, adxt_bdyt1, adxt_bdyt0);
        Two_One_Product(adxt_bdyt1, adxt_bdyt0, cdz, u3, u[2], u[1], u[0]);
        u[3] = u3;
        finlength = fast_expansion_sum_zeroelim(finlength, finnow, 4, u,
                                                finother);
        finswap = finnow; finnow = finother; finother = finswap;
        if (cdztail != 0.0) {
          Two_One_Product(adxt_bdyt1, adxt_bdyt0, cdztail, u3, u[2], u[1], u[0]);
          u[3] = u3;
          finlength = fast_expansion_sum_zeroelim(finlength, finnow, 4, u,
                                                  finother);
          finswap = finnow; finnow = finother; finother = finswap;
        }
      }
      if (cdytail != 0.0) {
        negate = -adxtail;
        Two_Product(negate, cdytail, adxt_cdyt1, adxt_cdyt0);
        Two_One_Product(adxt_cdyt1, adxt_cdyt0, bdz, u3, u[2], u[1], u[0]);
        u[3] = u3;
        finlength = fast_expansion_sum_zeroelim(finlength, finnow, 4, u,
                                                finother);
        finswap = finnow; finnow = finother; finother = finswap;
        if (bdztail != 0.0) {
          Two_One_Product(adxt_cdyt1, adxt_cdyt0, bdztail, u3, u[2], u[1], u[0]);
          u[3] = u3;
          finlength = fast_expansion_sum_zeroelim(finlength, finnow, 4, u,
                                                  finother);
          finswap = finnow; finnow = finother; finother = finswap;
        }
      }
    }
    if (bdxtail != 0.0) {
      if (cdytail != 0.0) {
        Two_Product(bdxtail, cdytail, bdxt_cdyt1, bdxt_cdyt0);
        Two_One_Product(bdxt_cdyt1, bdxt_cdyt0, adz, u3, u[2], u[1], u[0]);
        u[3] = u3;
        finlength = fast_expansion_sum_zeroelim(finlength, finnow, 4, u,
                                                finother);
        finswap = finnow; finnow = finother; finother = finswap;
        if (adztail != 0.0) {
          Two_One_Product(bdxt_cdyt1, bdxt_cdyt0, adztail, u3, u[2], u[1], u[0]);
          u[3] = u3;
          finlength = fast_expansion_sum_zeroelim(finlength, finnow, 4, u,
                                                  finother);
          finswap = finnow; finnow = finother; finother = finswap;
        }
      }
      if (adytail != 0.0) {
        negate = -bdxtail;
        Two_Product(negate, adytail, bdxt_adyt1, bdxt_adyt0);
        Two_One_Product(bdxt_adyt1, bdxt_adyt0, cdz, u3, u[2], u[1], u[0]);
        u[3] = u3;
        finlength = fast_expansion_sum_zeroelim(finlength, finnow, 4, u,
                                                finother);
        finswap = finnow; finnow = finother; finother = finswap;
        if (cdztail != 0.0) {
          Two_One_Product(bdxt_adyt1, bdxt_adyt0, cdztail, u3, u[2], u[1], u[0]);
          u[3] = u3;
          finlength = fast_expansion_sum_zeroelim(finlength, finnow, 4, u,
                                                  finother);
          finswap = finnow; finnow = finother; finother = finswap;
        }
      }
    }
    if (cdxtail != 0.0) {
      if (adytail != 0.0) {
        Two_Product(cdxtail, adytail, cdxt_adyt1, cdxt_adyt0);
        Two_One_Product(cdxt_adyt1, cdxt_adyt0, bdz, u3, u[2], u[1], u[0]);
        u[3] = u3;
        finlength = fast_expansion_sum_zeroelim(finlength, finnow, 4, u,
                                                finother);
        finswap = finnow; finnow = finother; finother = finswap;
        if (bdztail != 0.0) {
          Two_One_Product(cdxt_adyt1, cdxt_adyt0, bdztail, u3, u[2], u[1], u[0]);
          u[3] = u3;
          finlength = fast_expansion_sum_zeroelim(finlength, finnow, 4, u,
                                                  finother);
          finswap = finnow; finnow = finother; finother = finswap;
        }
      }
      if (bdytail != 0.0) {
        negate = -cdxtail;
        Two_Product(negate, bdytail, cdxt_bdyt1, cdxt_bdyt0);
        Two_One_Product(cdxt_bdyt1, cdxt_bdyt0, adz, u3, u[2], u[1], u[0]);
        u[3] = u3;
        finlength = fast_expansion_sum_zeroelim(finlength, finnow, 4, u,
                                                finother);
        finswap = finnow; finnow = finother; finother = finswap;
        if (adztail != 0.0) {
          Two_One_Product(cdxt_bdyt1, cdxt_bdyt0, adztail, u3, u[2], u[1], u[0]);
          u[3] = u3;
          finlength = fast_expansion_sum_zeroelim(finlength, finnow, 4, u,
                                                  finother);
          finswap = finnow; finnow = finother; finother = finswap;
        }
      }
    }

    if (adztail != 0.0) {
      wlength = scale_expansion_zeroelim(bctlen, bct, adztail, w);
      finlength = fast_expansion_sum_zeroelim(finlength, finnow, wlength, w,
                                              finother);
      finswap = finnow; finnow = finother; finother = finswap;
    }
    if (bdztail != 0.0) {
      wlength = scale_expansion_zeroelim(catlen, cat, bdztail, w);
      finlength = fast_expansion_sum_zeroelim(finlength, finnow, wlength, w,
                                              finother);
      finswap = finnow; finnow = finother; finother = finswap;
    }
    if (cdztail != 0.0) {
      wlength = scale_expansion_zeroelim(abtlen, abt, cdztail, w);
      finlength = fast_expansion_sum_zeroelim(finlength, finnow, wlength, w,
                                              finother);
      finswap = finnow; finnow = finother; finother = finswap;
    }

    return finnow[finlength - 1];
}

public:


UMBRA_FORCE_INLINE void init (void)
{
    if (g_predInitialized)
        return;

    REAL        half;
    REAL        check, lastcheck;
    int         every_other;

    every_other = 1;
    half        = 0.5f;
    epsilon     = 1.0f;
    splitter    = 1.0f;
    check       = 1.0f;

    /* Repeatedly divide `epsilon' by two until it is too small to add to    */
    /*   one without causing roundoff.  (Also check if the sum is equal to   */
    /*   the previous sum, for machines that round up instead of using exact */
    /*   rounding.  Not that this library will work on such machines anyway. */

    do
    {
        lastcheck = check;
        epsilon *= half;
        if (every_other)
        {
            splitter *= 2.0f;
        }
        every_other = !every_other;
        check = 1.0f + epsilon;
    } while ((check != 1.0) && (check != lastcheck));

    splitter += 1.0f;

    /* Error bounds for orientation and incircle tests. */
    resulterrbound = (3.0f + 8.0f * epsilon) * epsilon;
    ccwerrboundA = (3.0f + 16.0f * epsilon) * epsilon;
    ccwerrboundB = (2.0f + 12.0f * epsilon) * epsilon;
    ccwerrboundC = (9.0f + 64.0f * epsilon) * epsilon * epsilon;
    o3derrboundA = (7.0f + 56.0f * epsilon) * epsilon;
    o3derrboundB = (3.0f + 28.0f * epsilon) * epsilon;
    o3derrboundC = (26.0f + 288.0f * epsilon) * epsilon * epsilon;

    g_predInitialized = true;
}

UMBRA_FORCE_INLINE REAL orient2d(
    const double* pa,
    const double* pb,
    const double* pc) const
{
    REAL detleft, detright, det;
    REAL detsum, errbound;

    detleft = (pa[0] - pc[0]) * (pb[1] - pc[1]);
    detright = (pa[1] - pc[1]) * (pb[0] - pc[0]);
    det = detleft - detright;

    if (detleft > 0.0)
    {
        if (detright <= 0.0)
        {
            return det;
        } else
        {
            detsum = detleft + detright;
        }
    } else if (detleft < 0.0)
    {
        if (detright >= 0.0)
        {
            return det;
        } else
        {
            detsum = -detleft - detright;
        }
    } else
    {
      return det;
    }

    errbound = ccwerrboundA * detsum;
    if ((det >= errbound) || (-det >= errbound))
    {
        return det;
    }

    return orient2dadapt(pa, pb, pc, detsum);
}

REAL orient3dfast(
    const REAL* pa,
    const REAL* pb,
    const REAL* pc,
    const REAL* pd) const
{
    REAL adx, bdx, cdx;
    REAL ady, bdy, cdy;
    REAL adz, bdz, cdz;

    adx = pa[0] - pd[0];
    bdx = pb[0] - pd[0];
    cdx = pc[0] - pd[0];
    ady = pa[1] - pd[1];
    bdy = pb[1] - pd[1];
    cdy = pc[1] - pd[1];
    adz = pa[2] - pd[2];
    bdz = pb[2] - pd[2];
    cdz = pc[2] - pd[2];

    return adx * (bdy * cdz - bdz * cdy)
         + bdx * (cdy * adz - cdz * ady)
         + cdx * (ady * bdz - adz * bdy);
}

UMBRA_FORCE_INLINE REAL orient3d (
    const double* pa,
    const double* pb,
    const double* pc,
    const double* pd) const
{
    REAL cdx = pc[0] - pd[0];
    REAL bdx = pb[0] - pd[0];
    REAL cdy = pc[1] - pd[1];
    REAL bdy = pb[1] - pd[1];
    REAL adx = pa[0] - pd[0];
    REAL ady = pa[1] - pd[1];

    REAL bdxcdy = bdx * cdy;
    REAL cdxbdy = cdx * bdy;
    REAL cdxady = cdx * ady;
    REAL adxcdy = adx * cdy;

    REAL adxbdy = adx * bdy;
    REAL bdxady = bdx * ady;

    REAL adz = pa[2] - pd[2];
    REAL bdz = pb[2] - pd[2];
    REAL cdz = pc[2] - pd[2];

    REAL det =
          adz * (bdxcdy - cdxbdy)
        + bdz * (cdxady - adxcdy)
        + cdz * (adxbdy - bdxady);

    REAL permanent =
                (Absolute(bdxcdy) + Absolute(cdxbdy)) * Absolute(adz)
              + (Absolute(cdxady) + Absolute(adxcdy)) * Absolute(bdz)
              + (Absolute(adxbdy) + Absolute(bdxady)) * Absolute(cdz);

    if (Absolute(det) > (o3derrboundA * permanent))//(det > errbound) || (-det > errbound))
    {
        return det;
    }

    return orient3dadapt(pa, pb, pc, pd, permanent);
}

}; // class Pred

static Pred g_pred; // make sure everything gets initialized

/*----------------------------------------------------------------------*//*!
 * \brief   Converts a double-precision floating point into
 *          single-precision. If the result does not fit into the
 *          float's range, it is clamped to +-FLT_MAX. Same for
 *          underflows (note that underflows are _not_ clamped to zero,
 *          but instead to +- FLT_MIN).
 * \param   d       Double-precision floating point value
 * \return  Single-precision floating point value
 * \todo    This could probably be optimized somewhat (the ops could
 *          be done using integer code?)
 *//*----------------------------------------------------------------------*/

static UMBRA_FORCE_INLINE float doubleToFloatClamped (double d)
{
    double fd = std::fabs(d);

    if (fd >= FLT_MAX)                              // handle values that cannot be fit into floats
    {
        return d >= 0.0 ? FLT_MAX : -FLT_MAX;
    }

    if (fd <= FLT_MIN)
    {
        if (d == 0.0)
            return 0.0f;

        return d < 0.0 ? -FLT_MIN : +FLT_MIN;
    }

    return (float)(d);
}

float orient2dExact (
    const Vec2f& a,
    const Vec2f& b,
    const Vec2f& c)
{
    g_pred.init();

    double ad[2] = {a.x(), a.y()};
    double bd[2] = {b.x(), b.y()};
    double cd[2] = {c.x(), c.y()};
    float res1 = doubleToFloatClamped(g_pred.orient2d(ad, bd, cd));

    return res1;
}

float orient3dExact (
    const Vec3f& a,
    const Vec3f& b,
    const Vec3f& c,
    const Vec3f& d)
{
    g_pred.init();

    double ad[3] = {a.x(), a.y(), a.z()};
    double bd[3] = {b.x(), b.y(), b.z()};
    double cd[3] = {c.x(), c.y(), c.z()};
    double dd[3] = {d.x(), d.y(), d.z()};
    float res1 = doubleToFloatClamped(g_pred.orient3d(ad, bd, cd, dd));

    return res1;
}

}
