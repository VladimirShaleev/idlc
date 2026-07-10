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
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3001]: Syntax error at e3001:6:5");
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

TEST(idlc, DocumentationStringEmpty) {
    const auto [result, messages] = compile("e3016");
    GTEST_FAIL();
}

TEST(idlc, DetailAttrMustContainOneOrMoreArgs) {
    const auto [result, messages] = compile("e3017");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3017]: The [detail] attribute must contain one or more arguments at e3017:5:10");
}

TEST(idlc, InlineDocAllowedDetailOnlyAttr) {
    const auto [result, messages] = compile("e3018");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3018]: Inline documentation only [detail] description is allowed at e3018:5:37");
}

TEST(idlc, OrderAttrCanContainOneOptionalBoolParam) {
    const auto [result, messages] = compile("e3019");
    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0],
              "error [E3019]: The [order] attribute can contain one optional Boolean parameter at e3019:6:10");
    ASSERT_EQ(messages[1],
              "error [E3019]: The [order] attribute can contain one optional Boolean parameter at e3019:6:22");
    ASSERT_EQ(messages[2], "error [E3007]: Attribute duplication for attribute [order] in api 'Api' at e3019:6:22");
}

TEST(idlc, TabsNotAllowed) {
    const auto [result, messages] = compile("e3020");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3020]: Tabs are not allowed at e3020:6:4");
}

TEST(idlc, CouldNotFindFileForImport) {
    const auto [result, messages] = compile("e3021");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3021]: could not find file 'NotFoundImport' for import at e3021:9:1");
}

TEST(idlc, FailedOpenFile) {
    const auto [result, messages] = compile("e3022");
    GTEST_FAIL();
}

TEST(idlc, ConstCanBeDefinedOnlyForEnum) {
    const auto [result, messages] = compile("e3023");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3023]: A 'const' of 'Value' can be defined only for an 'enum' at e3023:7:5");
}

TEST(idlc, ValueAttrMustContainOneOrMoreArgs) {
    const auto [result, messages] = compile("e3024");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3024]: The [value] attribute must contain one or more arguments at e3024:10:18");
}

TEST(idlc, ValueAttrArgsMustBeLiteralsOrDeclReference) {
    const auto [result, messages] = compile("e3025");
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0], "error [E3002]: Argument parsing error 'fail' at e3025:10:24");
    ASSERT_EQ(messages[1], "error [E3025]: Arguments for the [value] attribute must be literals at e3025:10:18");
}

TEST(idlc, AllLiteralsInTheValueAttrMustBeOfSameType) {
    const auto [result, messages] = compile("e3026");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3026]: All literals in the [value] attribute must be of the same type at e3026:10:18");
}

TEST(idlc, TypeAttrArgCanOnlyReferToSymbols) {
    const auto [result, messages] = compile("e3027");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3027]: The [type] attribute argument can only refer to symbols at e3027:9:12");
}

TEST(idlc, CnameAttrMustContainSingleStringLiteralArg) {
    const auto [result, messages] = compile("e3028");
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0], "error [E3002]: Argument parsing error 'not string' at e3028:6:16");
    ASSERT_EQ(messages[1],
              "error [E3028]: The [cname] attribute must contain a single string literal argument at e3028:6:10");
}

TEST(idlc, CnameAttrMustSpecifyNameWithoutSpacesAndPuncts) {
    const auto [result, messages] = compile("e3029");
    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0],
              "error [E3029]: The [cname] attribute must specify a name (\"3.5\") without spaces and punctuations at "
              "e3029:6:10");
    ASSERT_EQ(messages[1],
              "error [E3029]: The [cname] attribute must specify a name (\"string with spaces\") without spaces and "
              "punctuations at e3029:10:19");
    ASSERT_EQ(messages[2],
              "error [E3029]: The [cname] attribute must specify a name (\"puncts!\") without spaces and punctuations "
              "at e3029:11:19");
}

TEST(idlc, SingleAttrCanContainOneOptionalBoolParam) {
    const auto [result, messages] = compile("e3030");
    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0],
              "error [E3030]: The [single] attribute can contain one optional Boolean parameter at e3030:6:10");
    ASSERT_EQ(messages[1],
              "error [E3030]: The [single] attribute can contain one optional Boolean parameter at e3030:6:23");
    ASSERT_EQ(messages[2], "error [E3007]: Attribute duplication for attribute [single] in api 'Api' at e3030:6:23");
}

TEST(idlc, InvalidTokenizerFormatString) {
    const auto [result, messages] = compile("e3031");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(
        messages[0],
        "error [E3031]: Invalid tokenizer format string \"2-a3-4\", a valid string looks like (2-^3-4) at e3031:6:10");
}

TEST(idlc, IntTokenizationParamsOrFmtStringMustBePassedToTokenizerAttr) {
    const auto [result, messages] = compile("e3032");
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0],
              "error [E3032]: Integer tokenization parameters or a tokenizer string must be passed to the attribute "
              "[tokenizer] at e3032:6:10");
    ASSERT_EQ(messages[1],
              "error [E3032]: Integer tokenization parameters or a tokenizer string must be passed to the attribute "
              "[tokenizer] at e3032:10:18");
}

TEST(idlc, TokenizerAttrMustContainOneOrMoreArgs) {
    const auto [result, messages] = compile("e3033");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3033]: The [tokenizer] attribute must contain one or more arguments (integers: 2, -2, 4 or "
              "string \"2-^3-4\") at e3033:6:10");
}

TEST(idlc, CopyrightAttrMustContainOneOrMoreArgs) {
    const auto [result, messages] = compile("e3034");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3034]: The [copyright] attribute must contain one or more arguments at e3034:5:10");
}

TEST(idlc, LicenseAttrMustContainOneOrMoreArgs) {
    const auto [result, messages] = compile("e3035");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3035]: The [license] attribute must contain one or more arguments at e3035:5:10");
}

TEST(idlc, IdentifiersCaseSensitive) {
    const auto [result, messages] = compile("e3036");
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0], "error [E3036]: Identifiers are case sensitive, error in 'ApI', but expected 'Api' at e3036:1:10");
    ASSERT_EQ(messages[1], "error [E3036]: Identifiers are case sensitive, error in 'Api.Test.value', but expected 'Api.Test.Value' at e3036:2:22");
}

TEST(idlc, SymbolDefinitionNotFound) {
    const auto [result, messages] = compile("e3037");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3037]: Symbol definition 'Test' not found at e3037:1:10");
}

TEST(idlc, ConstCanOnlyReferToOtherConstWhenEvaluated) {
    const auto [result, messages] = compile("e3038");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3038]: Constants can only refer to other constants when evaluated at e3038:10:5");
}

TEST(idlc, ConstCannotReferToItselfWhenEvaluated) {
    const auto [result, messages] = compile("e3039");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3039]: A constant 'Api.Test.Value' cannot refer to itself when evaluated at e3039:10:5");
}

TEST(idlc, EnumConstsCanOnlyBeSpecifiedAsIntOrEnumConsts) {
    const auto [result, messages] = compile("e3040");
    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0], "error [E3040]: Enumeration constants 'Api.Test.Value1' can only be specified as integers or enum consts at e3040:10:20");
    ASSERT_EQ(messages[1], "error [E3040]: Enumeration constants 'Api.Test.Value2' can only be specified as integers or enum consts at e3040:11:20");
    ASSERT_EQ(messages[2], "error [E3040]: Enumeration constants 'Api.Test.Value3' can only be specified as integers or enum consts at e3040:12:20");
}

TEST(idlc, FailedCalculateConst) {
    const auto [result, messages] = compile("e3041");
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0], "error [E3040]: Enumeration constants 'Api.Test.Value1' can only be specified as integers or enum consts at e3041:10:20");
    ASSERT_EQ(messages[1], "error [E3041]: Failed to calculate the constant 'Api.Test.Value2' at e3041:11:5");
}

TEST(idlc, CyclicDependenceOfConst) {
    const auto [result, messages] = compile("e3042");
    GTEST_FAIL();
}

TEST(idlc, TypeAttrMustContainOnlyOneType) {
    const auto [result, messages] = compile("e3043");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3043]: The [type] attribute must contain only one type at e3043:9:12");
}

TEST(idlc, EnumCanOnlyOfIntsType) {
    const auto [result, messages] = compile("e3044");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3044]: Enumeration 'Api.Test' can only of integers type at e3044:9:1");
}

TEST(idlc, EnumMustContainAtLeastOneConst) {
    const auto [result, messages] = compile("e3045");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3045]: Enumeration 'Api.Test' must contain at least one constant at e3045:9:1");
}
