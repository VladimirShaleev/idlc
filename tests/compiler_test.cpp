#include "compiler.hpp"

TEST(idlc, UnnecessaryParenthesesForAttribute) {
    const auto [result, messages] = compile("n1001.idl");

    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "note [N1001]: Unnecessary parentheses for a parameterless attribute [hex] at n1001:10:12");
}

TEST(idlc, EmptyAttributeList) {
    const auto [result, messages] = compile("n1002.idl");

    ASSERT_EQ(messages.size(), 4);
    ASSERT_EQ(messages[0], "note [N1002]: Unnecessary parentheses for empty attribute list at n1002:8:10");
    ASSERT_EQ(messages[1], "note [N1002]: Unnecessary parentheses for empty attribute list at n1002:10:11");
    ASSERT_EQ(messages[2], "note [N1002]: Unnecessary parentheses for empty attribute list at n1002:11:27");
    ASSERT_EQ(messages[3], "note [N1002]: Unnecessary parentheses for empty attribute list at n1002:11:17");
}

TEST(idlc, ExplicitAttributeBrief) {
    const auto [result, messages] = compile("n1003.idl");

    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "note [N1003]: Unnecessary explicit attribute [brief] in documentation at n1003:8:11");
}

TEST(idlc, ExplicitAttributeDetail) {
    const auto [result, messages] = compile("n1004.idl");

    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "note [N1004]: Unnecessary explicit attribute [detail] in inline documentation at n1004:11:25");
}

TEST(idlc, MissingAttribute) {
    const auto [result, messages] = compile("w2001.idl");

    ASSERT_EQ(messages.size(), 8);
    ASSERT_EQ(messages[0], "warning [W2001]: The declaration 'Api' is missing an attribute [brief] at w2001:1:1");
    ASSERT_EQ(messages[1], "warning [W2001]: The declaration 'Api' is missing an attribute [detail] at w2001:1:1");
    ASSERT_EQ(messages[2], "warning [W2001]: The declaration 'Api' is missing an attribute [author] at w2001:1:1");
    ASSERT_EQ(messages[3], "warning [W2001]: The declaration 'Api' is missing an attribute [copyright] at w2001:1:1");
    ASSERT_EQ(messages[4], "warning [W2001]: The declaration 'Api' is missing an attribute [license] at w2001:1:1");
    ASSERT_EQ(messages[5], "warning [W2001]: The declaration 'Api.Test' is missing an attribute [brief] at w2001:3:1");
    ASSERT_EQ(messages[6], "warning [W2001]: The declaration 'Api.Test' is missing an attribute [detail] at w2001:3:1");
    ASSERT_EQ(messages[7], "warning [W2001]: The declaration 'Api.Test.Value' is missing an attribute [detail] at w2001:4:5");
}

TEST(idlc, RepeatedImport) {
    const auto [result, messages] = compile("w2002.idl");

    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0], "warning [W2002]: Repeated import 'w2002' at w2002import:7:1");
    ASSERT_EQ(messages[1], "warning [W2002]: Repeated import 'w2002import' at w2002:17:1");
}

TEST(idlc, ConstantForwardRefers) {
    const auto [result, messages] = compile("w2003.idl");

    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "warning [W2003]: The constant 'Api.Test.Value3' refers to a constant declared below 'Api.Test.Value4' at w2003:13:5");
}

TEST(idlc, IntegerOutOfRange) {
    const auto [result, messages] = compile("w2004.idl");

    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0], "warning [W2004]: Integer Int8 with value 128 out of range [-128, 127] at w2004:12:5");
    ASSERT_EQ(messages[1], "warning [W2004]: Integer Int8 with value -130 out of range [-128, 127] at w2004:13:20");
}

TEST(idlc, SyntaxError) {
    const auto [result, messages] = compile("e3001.idl");
    GTEST_FAIL();
}

TEST(idlc, ArgumentParsingError) {
    const auto [result, messages] = compile("e3002.idl");

    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0], "error [E3002]: Argument parsing error 'asd fdfs' at e3002:6:32");
    ASSERT_EQ(messages[1], "warning [W2004]: Integer Int8 with value -130 out of range [-128, 127] at w2004:13:20");
}

TEST(idlc, VersionAttrRequiredParams) {
    const auto [result, messages] = compile("e3003.idl");
    GTEST_FAIL();
}

TEST(idlc, VersionComponentRange) {
    const auto [result, messages] = compile("e3004.idl");
    GTEST_FAIL();
}

TEST(idlc, InvalidAttrForDeclaration) {
    const auto [result, messages] = compile("e3005.idl");
    GTEST_FAIL();
}

TEST(idlc, AttrNotAllowedForDeclaration) {
    const auto [result, messages] = compile("e3006.idl");
    GTEST_FAIL();
}

TEST(idlc, attributeDuplication) {
    const auto [result, messages] = compile("e3007.idl");
    GTEST_FAIL();
}

TEST(idlc, AttributeMustNotHaveArguments) {
    const auto [result, messages] = compile("e3008.idl");
    GTEST_FAIL();
}

TEST(idlc, StringClosingCharacter) {
    const auto [result, messages] = compile("e3009.idl");
    GTEST_FAIL();
}

TEST(idlc, ApiRedeclaration) {
    const auto [result, messages] = compile("e3010.idl");
    GTEST_FAIL();
}

TEST(idlc, FirstDeclaration) {
    const auto [result, messages] = compile("e3011.idl");
    GTEST_FAIL();
}

TEST(idlc, SymbolRedefinition) {
    const auto [result, messages] = compile("e3012.idl");
    GTEST_FAIL();
}

TEST(idlc, UnknownAttribut) {
    const auto [result, messages] = compile("e3013.idl");
    GTEST_FAIL();
}

TEST(idlc, BriefAttrMustContainOneOrMoreArgs) {
    const auto [result, messages] = compile("e3014.idl");
    GTEST_FAIL();
}

TEST(idlc, UnknownAttributeInDoc) {
    const auto [result, messages] = compile("e3015.idl");
    GTEST_FAIL();
}
