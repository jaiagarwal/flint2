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

    Copyright (C) 2011 Fredrik Johansson
    Copyright (C) 2014 Abhinav Baid

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include "flint.h"
#include "fmpz.h"
#include "fmpz_mat.h"
#include "nmod_mat.h"
#include "ulong_extras.h"
#include "long_extras.h"

int
main(void)
{
    int i;
    FLINT_TEST_INIT(state);


    flint_printf("scalar_mul/tdiv_q_2exp....");
    fflush(stdout);

    for (i = 0; i < 100 * flint_test_multiplier(); i++)
    {
        fmpz_mat_t C, B, A;
        slong rows, cols;
        ulong exp;

        rows = n_randint(state, 10);
        cols = n_randint(state, 10);

        fmpz_mat_init(A, rows, cols);
        fmpz_mat_init(B, rows, cols);
        fmpz_mat_init(C, rows, cols);

        exp = n_randint(state, 200);
        fmpz_mat_randtest(A, state, 100);

        fmpz_mat_scalar_mul_2exp(B, A, exp);
        fmpz_mat_scalar_tdiv_q_2exp(C, B, exp);

        if (!fmpz_mat_equal(C, A))
        {
            flint_printf("FAIL!\n");
            abort();
        }

        fmpz_mat_clear(A);
        fmpz_mat_clear(B);
        fmpz_mat_clear(C);
    }

    /* test aliasing */
    for (i = 0; i < 100 * flint_test_multiplier(); i++)
    {
        fmpz_mat_t C, B, A;
        slong rows, cols;
        ulong exp;

        rows = n_randint(state, 10);
        cols = n_randint(state, 10);

        fmpz_mat_init(A, rows, cols);
        fmpz_mat_init(B, rows, cols);
        fmpz_mat_init(C, rows, cols);

        exp = n_randint(state, 200);
        fmpz_mat_randtest(A, state, 100);

        fmpz_mat_scalar_mul_2exp(B, A, exp);
        fmpz_mat_scalar_tdiv_q_2exp(C, B, exp);
        fmpz_mat_scalar_mul_2exp(A, A, exp);
        fmpz_mat_scalar_tdiv_q_2exp(A, A, exp);

        if (!fmpz_mat_equal(C, A))
        {
            flint_printf("FAIL!\n");
            abort();
        }

        fmpz_mat_clear(A);
        fmpz_mat_clear(B);
        fmpz_mat_clear(C);
    }

    FLINT_TEST_CLEANUP(state);
    flint_printf("PASS\n");
    return 0;
}
