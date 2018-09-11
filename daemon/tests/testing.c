#include "testing.h"

DEF_TEST(case1) {
        EXPECT_EQ_STR("testit", "testit");
        EXPECT_EQ_INT(1, 1);
        CHECK_NOT_NULL((void *)1);
        CHECK_NULL(NULL);
        EXPECT_EQ_UINT64(1, 1);
        EXPECT_EQ_DOUBLE(0.2, 0.2);
        CHECK_ZERO(0);

        return 0;
}

int main(void)
{
        RUN_TEST(case1);

        END_TEST;
}
