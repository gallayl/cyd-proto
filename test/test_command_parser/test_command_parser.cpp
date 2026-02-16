#include <unity.h>
#include "../../src/CommandInterpreter/CommandParser.h"

void test_get_command_name_single_word(void)
{
    TEST_ASSERT_EQUAL_STRING("foo", CommandParser::getCommandName("foo").c_str());
}

void test_get_command_name_multiple_words(void)
{
    TEST_ASSERT_EQUAL_STRING("foo", CommandParser::getCommandName("foo bar baz").c_str());
}

void test_get_parameter_0(void)
{
    TEST_ASSERT_EQUAL_STRING("foo", CommandParser::getCommandParameter("foo bar baz", 0).c_str());
}

void test_get_parameter_1(void)
{
    TEST_ASSERT_EQUAL_STRING("bar", CommandParser::getCommandParameter("foo bar baz", 1).c_str());
}

void test_get_parameter_2(void)
{
    TEST_ASSERT_EQUAL_STRING("baz", CommandParser::getCommandParameter("foo bar baz", 2).c_str());
}

void test_get_parameter_out_of_range(void)
{
    TEST_ASSERT_EQUAL_STRING("", CommandParser::getCommandParameter("foo bar baz", 3).c_str());
}

void test_get_parameter_empty_string(void)
{
    TEST_ASSERT_EQUAL_STRING("", CommandParser::getCommandParameter("", 0).c_str());
}

void test_get_parameter_nullptr(void)
{
    TEST_ASSERT_EQUAL_STRING("", CommandParser::getCommandParameter(nullptr, 0).c_str());
}

void test_get_command_name_with_trailing_spaces(void)
{
    TEST_ASSERT_EQUAL_STRING("foo", CommandParser::getCommandName("foo   ").c_str());
}

void test_get_parameter_multiple_spaces_between(void)
{
    TEST_ASSERT_EQUAL_STRING("bar", CommandParser::getCommandParameter("foo   bar", 1).c_str());
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_get_command_name_single_word);
    RUN_TEST(test_get_command_name_multiple_words);
    RUN_TEST(test_get_parameter_0);
    RUN_TEST(test_get_parameter_1);
    RUN_TEST(test_get_parameter_2);
    RUN_TEST(test_get_parameter_out_of_range);
    RUN_TEST(test_get_parameter_empty_string);
    RUN_TEST(test_get_parameter_nullptr);
    RUN_TEST(test_get_command_name_with_trailing_spaces);
    RUN_TEST(test_get_parameter_multiple_spaces_between);
    UNITY_END();
    return 0;
}
