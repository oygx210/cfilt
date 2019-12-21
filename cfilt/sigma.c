/**
 * Copyright 2020 Feras Boulala <ferasboulala@gmail.com>
 *
 * This file is part of cfilt.
 *
 * cfilt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cfilt is distributed in the hope it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cfilt. If not, see <https://www.gnu.org/licenses/>.
 */

#include "cfilt/sigma.h"
#include "cfilt/util.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_linalg.h>

#define M_ALLOC_ASSERT_VDM(p, n, m) M_ALLOC_ASSERT(p, n, m, cfilt_sigma_generator_van_der_merwe_free, *gen)
#define V_ALLOC_ASSERT_VDM(v, n) V_ALLOC_ASSERT(v, n, cfilt_sigma_generator_van_der_merwe_free, *gen)
#define VDM(n) (2 * (n) + 1)

static void
cfilt_sigma_generator_van_der_merwe_free(cfilt_sigma_generator_van_der_merwe *gen)
{
    M_FREE_IF_NOT_NULL(gen->_common.points);
    M_FREE_IF_NOT_NULL(gen->_chol);
    V_FREE_IF_NOT_NULL(gen->_common.mu_weights);
    V_FREE_IF_NOT_NULL(gen->_common.sigma_weights);

    free(gen);
}

static int
cfilt_sigma_generator_van_der_merwe_alloc(cfilt_sigma_generator_van_der_merwe **gen, const size_t n, const double alpha, const double beta, const double kappa)
{
    *gen = malloc(sizeof(cfilt_sigma_generator_van_der_merwe));
    if (*gen == NULL)
    {
        return GSL_ENOMEM;
    }

    cfilt_sigma_generator_van_der_merwe vdm;
    vdm._common.type = CFILT_VAN_DER_MERWE;
    vdm._common.n = n;

    vdm.alpha = alpha;
    vdm.beta = beta;
    vdm.kappa = kappa;
    vdm.lambda = pow(alpha, 2) * (n + kappa) - n;

    M_ALLOC_ASSERT_VDM(vdm._common.points, VDM(n), n);
    M_ALLOC_ASSERT_VDM(vdm._chol, n, n);
    V_ALLOC_ASSERT_VDM(vdm._common.mu_weights, VDM(n));
    V_ALLOC_ASSERT_VDM(vdm._common.sigma_weights, VDM(n));

    const double weight = 1.0 / 2.0 / n + vdm.lambda;
    gsl_vector_set_all(vdm._common.mu_weights, weight);
    gsl_vector_set_all(vdm._common.sigma_weights, weight);

    gsl_vector_set(vdm._common.mu_weights, 0, vdm.lambda / (vdm.lambda + n));
    gsl_vector_set(vdm._common.sigma_weights, 0, gsl_vector_get(vdm._common.mu_weights, 0) + 1 - pow(alpha, 2) + beta);

    memcpy(*gen, &vdm, sizeof(cfilt_sigma_generator_van_der_merwe));

    return GSL_SUCCESS;
}

static int
cfilt_sigma_generator_van_der_merwe_generate(cfilt_sigma_generator_van_der_merwe *gen, const gsl_vector *mu, const gsl_matrix *cov)
{
    // X_0
    memcpy(gen->_common.points->data, mu->data, gen->_common.n * mu->stride);

    // sqrt( (n + lambda) * cov )
    EXEC_ASSERT(gsl_matrix_memcpy, gen->_chol, cov);
    gsl_matrix_scale(gen->_chol, gen->_common.n + gen->lambda);
    EXEC_ASSERT(gsl_linalg_cholesky_decomp1, gen->_chol);

    // gsl will return a lower triangular matrix. We expect an upper one.
    EXEC_ASSERT(cfilt_matrix_tri_zero, gen->_chol, 1);

    // mu +/- variance
    for (size_t i = 0; i < gen->_common.n; ++i)
    {
        gsl_vector_view row = gsl_matrix_row(gen->_chol, i);
        gsl_vector *src = &row.vector;

        row = gsl_matrix_row(gen->_common.points, i + 1);
        gsl_vector *dst = &row.vector;

        // +
        gsl_vector_memcpy(dst, src);
        gsl_vector_add(dst, mu);

        row = gsl_matrix_row(gen->_common.points, i + 1 + gen->_common.n);
        dst = &row.vector;

        // -
        gsl_vector_memcpy(dst, src);
        gsl_vector_sub(dst, src);
        gsl_vector_scale(dst, -1);
    }

    return GSL_SUCCESS;
}

int
cfilt_sigma_generator_alloc(const cfilt_sigma_generator_type type, cfilt_sigma_generator** gen, const size_t n, ...)
{
    va_list valist;
    switch (type)
    {
        case CFILT_VAN_DER_MERWE:
            va_start(valist, n);

            const double alpha = va_arg(valist, double);
            const double beta = va_arg(valist, double);
            const double kappa = va_arg(valist, double);

            va_end(valist);

            EXEC_ASSERT(cfilt_sigma_generator_van_der_merwe_alloc, (cfilt_sigma_generator_van_der_merwe**)gen, n, alpha, beta, kappa);
            break;
        default:
            GSL_ERROR("Invalid sigma generator type", GSL_EINVAL);
    }

    return GSL_SUCCESS;
}

void
cfilt_sigma_generator_free(cfilt_sigma_generator* gen)
{
    switch (gen->type)
    {
        case CFILT_VAN_DER_MERWE:
            cfilt_sigma_generator_van_der_merwe_free((cfilt_sigma_generator_van_der_merwe*)gen);
            break;
     }
}

int cfilt_sigma_generator_generate(cfilt_sigma_generator* gen, const gsl_vector *mu, const gsl_matrix *cov)
{
    switch (gen->type)
    {
        case CFILT_VAN_DER_MERWE:
            EXEC_ASSERT(cfilt_sigma_generator_van_der_merwe_generate, (cfilt_sigma_generator_van_der_merwe*)gen, mu, cov);
            break;
        default:
            GSL_ERROR("Could not recognize sigma generator type", GSL_EINVAL);
    }

    return GSL_SUCCESS;
}