#ifndef _TEST_MAIN_TEST_HXX_
#define _TEST_MAIN_TEST_HXX_

#include <gtest/gtest.h>
#include <gmock/gmock.h>

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

std::string TEST_get_argument(std::string arg_prefix)
{
    for (const auto &s : ::testing::internal::GetArgvs())
    {
        if (s.substr(0, arg_prefix.size()) == arg_prefix)
        {
            return s.substr(arg_prefix.size());
        }
    }
    return "";
}
#endif // _TEST_MAIN_TEST_HXX_