#ifndef NK_TEST_H__ /*////////////////////////////////////////////////////////*/
#define NK_TEST_H__

#include "nk_define.h"

#if !defined(NK_TEST_MAX_FAILS)
#define NK_TEST_MAX_FAILS 64
#endif // !NK_TEST_MAX_FAILS

typedef struct nkTestFailure
{
    const nkChar* expression;
    const nkChar* file_name;
    nkS32         line_number;
}
nkTestFailure;

typedef struct nkTestContext
{
    const nkChar* name;
    nkBool        started;
    nkS32         checks_passed;
    nkS32         checks_performed;
    nkTestFailure failure[NK_TEST_MAX_FAILS]; // This is static just to avoid memory allocation. If you end up hitting the limit
}                                             // just define NK_TEST_MAX_FAILS to a larger value that fits your testing needs.
nkTestContext;

#define NK_TEST_BEGIN(ctx, name) nk_test_begin(ctx, name)
#define NK_TEST_END(  ctx)       nk_test_end  (ctx)
#define NK_TEST_CHECK(ctx, expr) nk_test_check(ctx, (expr), #expr, __FILE__, __LINE__)

NKAPI void nk_test_begin(nkTestContext* ctx, const nkChar* name);
NKAPI void nk_test_end  (nkTestContext* ctx);
NKAPI void nk_test_check(nkTestContext* ctx, nkBool expr, const nkChar* expr_str, const nkChar* file, nkS32 line);

/*============================================================================*/
/*============================== IMPLEMENTATION ==============================*/
/*============================================================================*/

#ifdef NK_TEST_IMPLEMENTATION /*//////////////////////////////////////////////*/

#include "nk_print.h"

NKAPI void nk_test_begin(nkTestContext* ctx, const nkChar* name)
{
    NK_ASSERT(ctx);
    NK_ASSERT(!ctx->started); // Finish the current test before starting another.

    nk_print_setup();

    ctx->name             = name;
    ctx->started          = NK_TRUE;
    ctx->checks_passed    = 0;
    ctx->checks_performed = 0;
}

NKAPI void nk_test_end(nkTestContext* ctx)
{
    NK_ASSERT(ctx);
    NK_ASSERT(ctx->started); // A test must be started before being ended.

    nkBool success = (ctx->checks_passed == ctx->checks_performed);

    if(success) nk_print_colored(nkTextColor_Default, nkTextColor_Green, "[PASS]: %s (%d/%d)\n", ctx->name, ctx->checks_passed, ctx->checks_performed);
    else nk_print_colored(nkTextColor_Default, nkTextColor_Red, "[FAIL]: %s (%d/%d)\n", ctx->name, ctx->checks_passed, ctx->checks_performed);

    // Print out information on all the checks that failed for the test.
    if(!success)
    {
        nkS32 checks_failed = (ctx->checks_performed - ctx->checks_passed);
        for(nkS32 i=0; i<checks_failed; ++i)
        {
            nkTestFailure* failure = &ctx->failure[i];
            nk_print_colored(nkTextColor_Default, nkTextColor_Red, "    expected (%s) in %s:%d\n",
                failure->expression, failure->file_name, failure->line_number);
        }
    }

    nk_print_shutdown();

    ctx->started = NK_FALSE;
}

NKAPI void nk_test_check(nkTestContext* ctx, nkBool expr, const nkChar* expr_str, const nkChar* file, nkS32 line)
{
    NK_ASSERT(ctx);
    NK_ASSERT(ctx->started); // A test must be started before being checked.

    // Log the stats on if we passed or failed the test.
    // If we fail store extra information about the fail.
    if(expr)
    {
        ctx->checks_passed++;
    }
    else
    {
        nkS32 index = (ctx->checks_performed - ctx->checks_passed);
        NK_ASSERT(index < NK_ARRAY_SIZE(ctx->failure));

        nkTestFailure* failure = &ctx->failure[index];

        failure->expression  = expr_str;
        failure->file_name   = file;
        failure->line_number = line;
    }

    // Update this at the end or else the index for storing failure info will be incorrect.
    ctx->checks_performed++;
}

#endif /* NK_TEST_IMPLEMENTATION /////////////////////////////////////////////*/

#endif /* NK_TEST_H__ ////////////////////////////////////////////////////////*/

/*******************************************************************************
 * MIT License
 *
 * Copyright (c) 2022-2023 Joshua Robertson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
*******************************************************************************/
