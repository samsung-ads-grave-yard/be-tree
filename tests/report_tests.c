#include <stdbool.h>
#include <stdio.h>

#include "ast.h"
#include "betree.h"
#include "minunit.h"
#include "tree.h"

int test_integer()
{
    struct betree* tree = betree_make();
    tree->config->lnode_max_cap = 1;
    add_attr_domain_bounded_i(tree->config, "a", false, 0, 10);

    {
        struct report* report = make_report();
        mu_assert(betree_insert(tree, 1, "a > 6"), "");
        mu_assert(betree_insert(tree, 2, "a < 6"), "");
        mu_assert(betree_search(tree, "{\"a\":2}", report), "");
        mu_assert(report->evaluated == 1 && report->matched == 1, "");
        free_report(report);
    }

    betree_free(tree);

    return 0;
}

int test_float()
{
    struct betree* tree = betree_make();
    tree->config->lnode_max_cap = 1;
    add_attr_domain_bounded_f(tree->config, "f", false, 0., 10.);

    {
        struct report* report = make_report();
        mu_assert(betree_insert(tree, 1, "f > 6."), "");
        mu_assert(betree_insert(tree, 2, "f < 6."), "");
        mu_assert(betree_search(tree, "{\"f\":2.}", report), "");
        mu_assert(report->evaluated == 1 && report->matched == 1, "");
        free_report(report);
    }

    betree_free(tree);

    return 0;
}

int test_string()
{
    struct betree* tree = betree_make();
    tree->config->lnode_max_cap = 1;
    add_attr_domain_s(tree->config, "s", false);

    {
        struct report* report = make_report();
        mu_assert(betree_insert(tree, 1, "s = \"1\""), "");
        mu_assert(betree_insert(tree, 2, "s = \"2\""), "");
        mu_assert(betree_search(tree, "{\"s\":\"1\"}", report), "");
        mu_assert(report->evaluated == 2 && report->matched == 1, "");
        free_report(report);
    }

    betree_free(tree);

    return 0;
}

int all_tests()
{
    mu_run_test(test_integer);
    mu_run_test(test_float);
    mu_run_test(test_string);

    return 0;
}

RUN_TESTS()
