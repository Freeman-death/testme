#include "../core/protocol.h"
#include "proto.h"
/* ctype tests — owned by subagent 1. One example to prove the pipeline. */
static void test_isalpha_basic(void) {
    expect_true("ft_isalpha('a')", ft_isalpha('a') == 1, __FILE__, __LINE__);
    expect_true("ft_isalpha('0') == 0", ft_isalpha('0') == 0, __FILE__, __LINE__);
}
TEST("ft_isalpha basic", test_isalpha_basic);
