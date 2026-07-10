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
    ASSERT_EQ(messages[0],
              "note [N1004]: Unnecessary explicit attribute [detail] in inline documentation at n1004:11:25");
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
    ASSERT_EQ(messages[7],
              "warning [W2001]: The declaration 'Api.Test.Value' is missing an attribute [detail] at w2001:4:5");
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
    ASSERT_EQ(messages[0],
              "warning [W2003]: The constant 'Api.Test.Value3' refers to a constant declared below 'Api.Test.Value4' "
              "at w2003:13:5");
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
    ASSERT_EQ(messages[0], "error [E3002]: Argument parsing error 'invalid arg' at e3002:6:24");
    ASSERT_EQ(messages[1],
              "error [E3003]: The [version] attribute must have three required integer parameters, such as version(1, "
              "2, 3) or version(\"string\") at e3002:6:10");
}

TEST(idlc, VersionAttrRequiredParams) {
    const auto [result, messages] = compile("e3003.idl");

    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3003]: The [version] attribute must have three required integer parameters, such as version(1, "
              "2, 3) or version(\"string\") at e3003:6:10");
}

TEST(idlc, VersionComponentRange) {
    const auto [result, messages] = compile("e3004.idl");

    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0],
              "error [E3004]: Version values must be between 0 and 255, while the argument is -1 at e3004:6:10");
    ASSERT_EQ(messages[1],
              "error [E3004]: Version values must be between 0 and 255, while the argument is 256 at e3004:6:10");
    ASSERT_EQ(messages[2],
              "error [E3004]: Version values must be between 0 and 255, while the argument is 1000 at e3004:6:10");
}

TEST(idlc, InvalidAttrForDeclaration) {
    const auto [result, messages] = compile("e3005.idl");

    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0],
              "error [E3005]: Invalid attribute [flags] for api 'Api' declaration, allowed attributes are cname, "
              "tokenizer, order, single, version, brief, detail, author, copyright, license at e3005:6:10");
    ASSERT_EQ(messages[1],
              "error [E3005]: Invalid attribute [hex] for api 'Api' declaration, allowed attributes are cname, "
              "tokenizer, order, single, version, brief, detail, author, copyright, license at e3005:6:17");
}

TEST(idlc, AttrNotAllowedForDeclaration) {
    const auto [result, messages] = compile("e3006.idl");
    GTEST_FAIL();
}

TEST(idlc, AttributeDuplication) {
    const auto [result, messages] = compile("e3007.idl");

    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3007]: Attribute duplication for attribute [version] in api 'Api' at e3007:6:28");
}

TEST(idlc, AttributeMustNotHaveArguments) {
    const auto [result, messages] = compile("e3008.idl");

    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3008]: The attribute [hex] must not have arguments at e3008:10:12");
}

TEST(idlc, StringClosingCharacter) {
    const auto [result, messages] = compile("e3009.idl");

    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(
        messages[0],
        "error [E3009]: String closing character not found in string: \"Lost closing character)])] at e3009:5:17");
    ASSERT_EQ(messages[1], "error [E3001]: Syntax error at e3009:5:45");
}

TEST(idlc, ApiRedeclaration) {
    const auto [result, messages] = compile("e3010.idl");

    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3010]: API Redeclaration 'Other' at e3010:17:1");
}

TEST(idlc, FirstDeclaration) {
    const auto [result, messages] = compile("e3011.idl");

    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3011]: The first declaration in the description should always begin with the 'api' declaration");
}

TEST(idlc, SymbolRedefinition) {
    const auto [result, messages] = compile("e3012.idl");

    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0], "error [E3012]: Symbol redefinition 'Api.Test.Value' at e3012:16:5");
    ASSERT_EQ(messages[1], "error [E3012]: Symbol redefinition 'Api.Test.TeSt' at e3012:17:5");
}

TEST(idlc, UnknownAttribute) {
    const auto [result, messages] = compile("e3013.idl");

    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0], "error [E3013]: Unknown attribute [abcd] at e3013:6:13");
    ASSERT_EQ(messages[1], "error [E3013]: Unknown attribute [invalidattr] at e3013:7:31");
    ASSERT_EQ(messages[2], "error [E3013]: Unknown attribute [xyz] at e3013:7:69");
}

TEST(idlc, BriefAttrMustContainOneOrMoreArgs) {
    const auto [result, messages] = compile("e3014.idl");

    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3014]: The [brief] attribute must contain one or more arguments at e3014:5:10");
}

TEST(idlc, UnknownAttributeInDoc) {
    const auto [result, messages] = compile("e3015.idl");

    ASSERT_EQ(messages.size(), 7);
    ASSERT_EQ(messages[0], "error [E3008]: The attribute [flags] must not have arguments at e3015:6:31");
    ASSERT_EQ(messages[1], "error [E3015]: Unknown attribute in the documentation [flags] at e3015:6:31");
    ASSERT_EQ(messages[2], "error [E3008]: The attribute [hex] must not have arguments at e3015:7:39");
    ASSERT_EQ(messages[3], "error [E3015]: Unknown attribute in the documentation [hex] at e3015:7:39");
    ASSERT_EQ(messages[4], "error [E3018]: Inline documentation only [detail] description is allowed at e3015:7:39");
    ASSERT_EQ(messages[5],
              "error [E3005]: Invalid attribute [flags] for api 'Api' declaration, allowed attributes are cname, "
              "tokenizer, order, single, version, brief, detail, author, copyright, license at e3015:6:31");
    ASSERT_EQ(messages[6],
              "error [E3005]: Invalid attribute [hex] for api 'Api' declaration, allowed attributes are cname, "
              "tokenizer, order, single, version, brief, detail, author, copyright, license at e3015:7:39");
}
