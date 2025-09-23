#include "test/main_Test.hxx"

#include "utilities/mustangpeak_string_helper.h"

TEST(StringHelper, strnew)
{
    // This is a difficult test because the underlying implementation is
    // standard malloc. Therefore it is difficult to validate the results. The
    // following provides some sanity, but is not comprehensive.
    char *new_str = strnew(4);
    strcpy(new_str, "test");
    EXPECT_EQ(0, strcmp(new_str, "test"));
    free(new_str);
}

TEST(StringHelper, strnew_initialized)
{
    char *new_str = strnew_initialized(4);
    EXPECT_EQ(new_str[0], '\0');
    EXPECT_EQ(new_str[1], '\0');
    EXPECT_EQ(new_str[2], '\0');
    EXPECT_EQ(new_str[3], '\0');
    EXPECT_EQ(new_str[4], '\0');
    free(new_str);
}

TEST(StringHelper, strcatnew)
{
    std::string str1 = "str1";
    std::string str2 = "str2";
    char *new_str = strcatnew((char*)str1.c_str(), (char*)str2.c_str());
    EXPECT_EQ("str1str2", std::string(new_str));
    free(new_str);
}