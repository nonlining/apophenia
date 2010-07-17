/** \file apop_zipf.c

  The Zipf distribution.

\f$Z(a)        = {1\over \zeta(a) * i^a}        \f$<br>
*/ /* Copyright (c) 2005--2009 by Ben Klemens.  Licensed under the modified GNU GPL v2; see COPYING and COPYING2.  */

#include "mapply.h"
#include "likelihoods.h"
#include <gsl/gsl_sf_zeta.h>
#include "internal.h"

static double beta_greater_than_x_constraint(apop_data *returned_beta, apop_model *m){
    //constraint is 1 < beta_1
  Nullcheck_m(m); Nullcheck_p(m);
  static apop_data *constraint = NULL;
    if (!constraint) {
        constraint = apop_data_calloc(1,1,1);
        apop_data_set(constraint, 0, 0, 1);
        apop_data_set(constraint, 0, -1, 1);
    }
    return apop_linear_constraint(m->parameters->vector, constraint, 1e-4);
}

static double zipf_log_likelihood(apop_data *d, apop_model *m){
  Get_vmsizes(d) //tsize
  Nullcheck(d); Nullcheck_m(m); Nullcheck_p(m);
  long double   bb      = gsl_vector_get(m->parameters->vector, 0);
  double like = -apop_map_sum(d, log);
    like    *= bb;
    like    -= log(gsl_sf_zeta(bb)) * tsize;
    return like;
}    

/** Draw from a Zipf distribution with parameter \f$ a \f$

Call this fn using \ref apop_zipf.rng().

Returns a ranking: If the population were Zipf distributed, you're most
likely to get the 1st most common item, so this produces a lot of ones,
a great deal of twos, and so on.

In the interest of avoiding overflows, the RNG is capped at 1e8.

For example:
\code
gsl_rng *r  = apop_rng_alloc(r);
double  d   = 1.4
gsl_vector *params = apop_array_to_vector(&d,1);
apop_zipf.draw(r, 1.4, NULL);
\endcode

Cribbed from <a href="http://cgm.cs.mcgill.ca/~luc/mbookindex.html>Devroye (1986)</a>, Chapter 10, p 551.  */
static void zipf_rng(double *out, gsl_rng* r, apop_model *param){
  double a  = gsl_vector_get(param->parameters->vector, 0);
  apop_assert_s(a >= 1, "Zipf needs a parameter >=1. Stopping."); 
  int     x;
  double  u, v, t, 
          b       = pow(2, a-1), 
          ainv    = -(1.0/(a-1));
    do {
        u    = gsl_rng_uniform(r);
        v    = gsl_rng_uniform(r);
        x    = pow(u,ainv);
        t    = pow((1.0 + 1.0/x), (a-1));
    } while (v * x * (t-1.0)/(b-1) > t/b);
    assert(x < 1e5);
    *out = x;
}

apop_model apop_zipf = {"Zipf", 1,0,0, .dsize=1,
     .log_likelihood = zipf_log_likelihood, .constraint = beta_greater_than_x_constraint, .draw = zipf_rng};
