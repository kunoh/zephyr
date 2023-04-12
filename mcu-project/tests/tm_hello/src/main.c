#include <zephyr/ztest.h>
#include "tm_hello_module.h"

void test_return_arg(void)
{
    zassert_equal(return_arg(7), 7, "return_aarg unexpected result");
}

void test_main(void)
{
	ztest_test_suite(tm_hello_module_unit_tests,
			 ztest_unit_test(test_return_arg)
	);
	ztest_run_test_suite(tm_hello_module_unit_tests);
}
