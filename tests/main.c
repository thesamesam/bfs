// Copyright © Tavian Barnes <tavianator@tavianator.com>
// SPDX-License-Identifier: 0BSD

/**
 * Entry point for unit tests.
 */

#include "tests.h"
#include "../src/bfstd.h"
#include "../src/color.h"
#include "../src/config.h"
#include "../src/diag.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Test context.
 */
struct test_ctx {
	/** Number of command line arguments. */
	int argc;
	/** The arguments themselves. */
	char **argv;

	/** Parsed colors. */
	struct colors *colors;
	/** Colorized output stream. */
	CFILE *cout;

	/** Eventual exit status. */
	int ret;
};

/** Initialize the test context. */
static int test_init(struct test_ctx *ctx, int argc, char **argv) {
	ctx->argc = argc;
	ctx->argv = argv;

	ctx->colors = parse_colors();
	ctx->cout = cfwrap(stdout, ctx->colors, false);
	if (!ctx->cout) {
		ctx->ret = EXIT_FAILURE;
		return -1;
	}

	ctx->ret = EXIT_SUCCESS;
	return 0;
}

/** Finalize the test context. */
static int test_fini(struct test_ctx *ctx) {
	if (ctx->cout) {
		cfclose(ctx->cout);
	}

	free_colors(ctx->colors);

	return ctx->ret;
}

/** Check if a test case is enabled for this run. */
static bool should_run(const struct test_ctx *ctx, const char *test) {
	// Run all tests by default
	if (ctx->argc < 2) {
		return true;
	}

	// With args, run only specified tests
	for (int i = 1; i < ctx->argc; ++i) {
		if (strcmp(test, ctx->argv[i]) == 0) {
			return true;
		}
	}

	return false;
}

/** Run a test if it's enabled. */
static void run_test(struct test_ctx *ctx, const char *test, test_fn *fn) {
	if (should_run(ctx, test)) {
		if (fn()) {
			cfprintf(ctx->cout, "${grn}[PASS]${rs} ${bld}%s${rs}\n", test);
		} else {
			cfprintf(ctx->cout, "${red}[FAIL]${rs} ${bld}%s${rs}\n", test);
			ctx->ret = EXIT_FAILURE;
		}
	}
}

int main(int argc, char *argv[]) {
	struct test_ctx ctx;
	if (test_init(&ctx, argc, argv) != 0) {
		goto done;
	}

	run_test(&ctx, "alloc", check_alloc);
	run_test(&ctx, "bfstd", check_bfstd);
	run_test(&ctx, "bit", check_bit);
	run_test(&ctx, "trie", check_trie);
	run_test(&ctx, "xtime", check_xtime);

done:
	return test_fini(&ctx);
}
