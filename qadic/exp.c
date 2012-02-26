/*=============================================================================

    This file is part of FLINT.

    FLINT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    FLINT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FLINT; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

=============================================================================*/
/******************************************************************************

    Copyright (C) 2012 Sebastian Pancratz
 
******************************************************************************/

#include "fmpz_mod_poly.h"
#include "qadic.h"

extern long _padic_exp_bound(long v, long N, const fmpz_t p);

void _qadic_exp(fmpz *rop, const fmpz *op, long v, long len, 
                const fmpz *a, const long *j, long lena, 
                const fmpz_t p, long N)
{
    const long d = j[lena - 1];
    const long n = _padic_exp_bound(v, N, p);

    fmpz_t pN;

    fmpz_init(pN);
    fmpz_pow_ui(pN, p, N);

    if (d == 1)
    {
        /* FIXME:  Rewrite _padic_exp signature without ctx, call from here */
    }
    else if (n < 4)
    {
        if (n == 1)  /* y := 1 */
        {
            fmpz_one(rop);
            _fmpz_vec_zero(rop + 1, d - 1);
        }
        else if (n == 2)  /* y := 1 + x */
        {
            fmpz_t f;

            fmpz_init(f);
            fmpz_pow_ui(f, p, v);

            _fmpz_vec_scalar_mul_fmpz(rop, op, len, f);
            _fmpz_vec_zero(rop + len, d - len);
            fmpz_add_ui(rop, rop, 1);
            _fmpz_vec_scalar_mod_fmpz(rop, rop, len, pN);

            fmpz_clear(f);
        }
        else  /* y := 1 + x + x^2/2 */
        {
            long i;
            fmpz *x = _fmpz_vec_init(len + 1);

            fmpz_pow_ui(x + len, p, v);
            _fmpz_vec_scalar_mul_fmpz(x, op, len, x + len);

            _fmpz_poly_sqr(rop, x, len);
            if (*p != 2L)
            {
                for (i = 0; i < 2 * len - 1; i++)
                    if (fmpz_is_odd(rop + i))
                        fmpz_add(rop + i, rop + i, pN);
            }
            _fmpz_vec_scalar_fdiv_q_2exp(rop, rop, 2 * len - 1, 1);
            _fmpz_mod_poly_reduce(rop, 2 * len - 1, a, j, lena, pN);
            _fmpz_mod_poly_add(rop, rop, d, x, len, pN);
            fmpz_add_ui(rop, rop, 1);
            if (fmpz_equal(rop, pN))
                fmpz_zero(rop);

            _fmpz_vec_clear(x, len + 1);
        }
    }
    else  /* d >= 1, n >= 4 */
    {
        const long k = fmpz_fits_si(p) ? 
                       (n - 1 - 1) / (fmpz_get_si(p) - 1) : 0;
        const long b = n_sqrt(n);

        long i;
        fmpz_t c, f, pNk;
        fmpz *s, *t, *x;

        fmpz_init(c);
        fmpz_init(f);
        fmpz_init(pNk);

        s = _fmpz_vec_init(2 * d - 1);
        t = _fmpz_vec_init(2 * d - 1);
        x = _fmpz_vec_init(d * (b + 1) + d - 1);

        fmpz_pow_ui(f, p, v);
        fmpz_pow_ui(pNk, p, N + k);

        /* Compute powers x^i of the argument */
        fmpz_one(x);
        _fmpz_vec_scalar_mul_fmpz(x + d, op, len, f);
        _fmpz_vec_zero(x + d + len, d - len);
        for (i = 2; i <= b; i++)
        {
            _fmpz_mod_poly_mul(x + i * d, x + (i - 1) * d, d, x + d, d, pNk);
            _fmpz_mod_poly_reduce(x + i * d, 2 * d - 1, a, j, lena, pNk);
        }

        _fmpz_vec_zero(rop, d);
        fmpz_one(f);

        for (i = (n + b - 1) / b - 1; i >= 0; i--)
        {
            long lo = i * b;
            long hi = FLINT_MIN(n - 1, lo + b - 1);

            _fmpz_vec_set(s, x + (hi - lo) * d, d);
            fmpz_set_ui(c, hi);
            hi--;

            for ( ; hi > lo; hi--)
            {
                _fmpz_vec_scalar_addmul_fmpz(s, x + (hi - lo) * d, d, c);
                fmpz_mul_ui(c, c, hi);
            }

            if (hi == lo)
            {
                fmpz_add(s, s, c);
                if (hi != 0)
                    fmpz_mul_ui(c, c, hi);
            }

            _fmpz_poly_mul(t, x + b * d, d, rop, d);
            _fmpz_mod_poly_reduce(t, 2 * d - 1, a, j, lena, pNk);
            _fmpz_vec_scalar_mul_fmpz(rop, s, d, f);
            _fmpz_vec_add(rop, rop, t, d);
            _fmpz_vec_scalar_mod_fmpz(rop, rop, d, pNk);

            fmpz_mul(f, f, c);
        }

        /* Note exp(x) is a unit so val(sum) == val(f). */
        i = fmpz_remove(f, f, p);
        if (i)
        {
            fmpz_pow_ui(c, p, i);
            _fmpz_vec_scalar_divexact_fmpz(rop, rop, d, c);
        }

        _padic_inv(f, f, p, N);
        _fmpz_vec_scalar_mul_fmpz(rop, rop, d, f);
        _fmpz_vec_scalar_mod_fmpz(rop, rop, d, pN);

        _fmpz_vec_clear(s, 2 * d - 1);
        _fmpz_vec_clear(t, 2 * d - 1);
        _fmpz_vec_clear(x, d * (b + 1) + d - 1);
        fmpz_clear(c);
        fmpz_clear(f);
        fmpz_clear(pNk);
    }

    fmpz_clear(pN);
}

int qadic_exp(qadic_t rop, const qadic_t op, const qadic_ctx_t ctx)
{
    const long N  = (&ctx->pctx)->N;
    const long v  = op->val;
    const fmpz *p = (&ctx->pctx)->p;

    if (padic_poly_is_zero(op))
    {
        padic_poly_one(rop, &ctx->pctx);
        return 1;
    }

    if ((*p == 2L && v <= 1) || (v <= 0))
    {
        return 0;
    }
    else
    {
        if (v < N)
        {
            const long d = qadic_ctx_degree(ctx);
            fmpz *t;

            if (rop != op)
            {
                padic_poly_fit_length(rop, 2 * d - 1);
                t = rop->coeffs;
            }
            else
            {
                t = _fmpz_vec_init(2 * d - 1);
            }

            _qadic_exp(t, op->coeffs, v, op->length, 
                       ctx->a, ctx->j, ctx->len, p, N);
            rop->val = 0;

            if (rop == op)
            {
                _fmpz_vec_clear(rop->coeffs, rop->alloc);
                rop->coeffs = t;
                rop->alloc  = 2 * d - 1;
                rop->length = d;
            }
            _padic_poly_set_length(rop, d);
            _padic_poly_normalise(rop);
        }
        else
            padic_poly_one(rop, &ctx->pctx);
        return 1;
    }
}
