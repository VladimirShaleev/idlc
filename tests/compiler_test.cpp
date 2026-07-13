#include "compiler.hpp"

TEST(idlc, UnnecessaryParenthesesForAttribute) {
    const auto [result, ast, messages] = compile("n1001.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "note [N1001]: Unnecessary parentheses for a parameterless attribute [hex] at n1001:10:12");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);
    ASSERT_EQ(getStr(ast, test), "Test");

    auto hex = findChild(ast, test, IDL_AST_NODE_TYPE_ATTR_HEX);
    ASSERT_NE(hex, HandleNone);
}

TEST(idlc, EmptyAttributeList) {
    const auto [result, ast, messages] = compile("n1002.idl");
    deferred(idl_compilation_result_destroy(ast));

    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(messages.size(), 4);
    ASSERT_EQ(messages[0], "note [N1002]: Unnecessary parentheses for empty attribute list at n1002:8:10");
    ASSERT_EQ(messages[1], "note [N1002]: Unnecessary parentheses for empty attribute list at n1002:10:11");
    ASSERT_EQ(messages[2], "note [N1002]: Unnecessary parentheses for empty attribute list at n1002:11:27");
    ASSERT_EQ(messages[3], "note [N1002]: Unnecessary parentheses for empty attribute list at n1002:11:17");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);
    ASSERT_EQ(getStr(ast, test), "Test");

    auto testAttrs = getAttrs(ast, test);
    ASSERT_EQ(testAttrs.size(), 3);
    ASSERT_TRUE(isType(ast, testAttrs[0], IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF));
    ASSERT_TRUE(isType(ast, testAttrs[1], IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL));
    ASSERT_TRUE(isType(ast, testAttrs[2], IDL_AST_NODE_TYPE_ATTR_TYPE));

    auto value = findChild(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_NE(value, HandleNone);

    auto valueAttrs = getAttrs(ast, value);
    ASSERT_EQ(valueAttrs.size(), 2);
    ASSERT_TRUE(isType(ast, valueAttrs[0], IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL));
    ASSERT_TRUE(isType(ast, valueAttrs[1], IDL_AST_NODE_TYPE_ATTR_VALUE));
}

TEST(idlc, ExplicitAttributeBrief) {
    const auto [result, ast, messages] = compile("n1003.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "note [N1003]: Unnecessary explicit attribute [brief] in documentation at n1003:8:11");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);
    ASSERT_EQ(getStr(ast, test), "Test");

    auto brief = findChild(ast, test, IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF);
    ASSERT_NE(brief, HandleNone);
}

TEST(idlc, ExplicitAttributeDetail) {
    const auto [result, ast, messages] = compile("n1004.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "note [N1004]: Unnecessary explicit attribute [detail] in inline documentation at n1004:11:25");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);
    ASSERT_EQ(getStr(ast, test), "Test");

    auto value = findChild(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_NE(value, HandleNone);

    auto detail = findChild(ast, value, IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL);
    ASSERT_NE(detail, HandleNone);
}

TEST(idlc, MissingAttribute) {
    const auto [result, ast, messages] = compile("w2001.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
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

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);
    ASSERT_EQ(getAttrs(ast, api, AttrFilterDoc).size(), 0);
    ASSERT_EQ(getAttrs(ast, api).size(), 0);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);
    ASSERT_EQ(getStr(ast, test), "Test");
    ASSERT_EQ(getAttrs(ast, test, AttrFilterDoc).size(), 0);
    ASSERT_EQ(getAttrs(ast, test).size(), 1);
    ASSERT_TRUE(isType(ast, getAttrs(ast, test)[0], IDL_AST_NODE_TYPE_ATTR_TYPE));

    auto value = findChild(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_NE(value, HandleNone);
    ASSERT_EQ(getStr(ast, value), "Value");
    ASSERT_EQ(getAttrs(ast, value, AttrFilterDoc).size(), 0);
    ASSERT_EQ(getAttrs(ast, value).size(), 1);
    ASSERT_TRUE(isType(ast, getAttrs(ast, value)[0], IDL_AST_NODE_TYPE_ATTR_VALUE));
}

TEST(idlc, RepeatedImport) {
    const auto [result, ast, messages] = compile("w2002.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0], "warning [W2002]: Repeated import 'w2002' at w2002import:7:1");
    ASSERT_EQ(messages[1], "warning [W2002]: Repeated import 'w2002import' at w2002:17:1");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto imports = getChilds(ast, api, IDL_AST_NODE_TYPE_IMPORT);
    ASSERT_EQ(imports.size(), 1);

    auto w2002Import = imports[0];
    ASSERT_NE(w2002Import, HandleNone);
    ASSERT_EQ(getStr(ast, w2002Import), "W2002Import");

    auto emptyImports = getChilds(ast, w2002Import, IDL_AST_NODE_TYPE_IMPORT);
    ASSERT_EQ(emptyImports.size(), 0);
}

TEST(idlc, ConstantForwardRefers) {
    const auto [result, ast, messages] = compile("w2003.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "warning [W2003]: The constant 'Api.Test.Value3' refers to a constant declared below 'Api.Test.Value4' "
              "at w2003:13:5");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto testConsts = getChilds(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_EQ(testConsts.size(), 4);
    ASSERT_TRUE(checkConst(ast, testConsts[0], 0, true, false, false));
    ASSERT_TRUE(checkConst(ast, testConsts[1], 0, false, false, false));
    ASSERT_TRUE(checkConst(ast, testConsts[2], 5, false, false, true));
    ASSERT_TRUE(checkConst(ast, testConsts[3], 5, false, false, false));
}

TEST(idlc, IntegerOutOfRange) {
    const auto [result, ast, messages] = compile("w2004.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0], "warning [W2004]: Integer Int8 with value 128 out of range [-128, 127] at w2004:19:5");
    ASSERT_EQ(messages[1], "warning [W2004]: Integer Int8 with value -130 out of range [-128, 127] at w2004:20:20");
    ASSERT_EQ(messages[2], "warning [W2004]: Integer Int8 with value 367 out of range [-128, 127] at w2004:22:20");
    ASSERT_FALSE(idl_compilation_result_has_notes(ast));
    ASSERT_TRUE(idl_compilation_result_has_warnings(ast));
    ASSERT_FALSE(idl_compilation_result_has_errors(ast));

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto enums = getChilds(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_EQ(enums.size(), 2);

    auto firstConsts = getChilds(ast, enums[0], IDL_AST_NODE_TYPE_CONST);
    ASSERT_EQ(firstConsts.size(), 3);
    ASSERT_TRUE(checkConst(ast, firstConsts[0], 100, false, false, false));
    ASSERT_TRUE(checkConst(ast, firstConsts[1], 300, false, false, false));
    ASSERT_TRUE(checkConst(ast, firstConsts[2], 367, false, false, false));

    auto secondConsts = getChilds(ast, enums[1], IDL_AST_NODE_TYPE_CONST);
    ASSERT_EQ(secondConsts.size(), 5);
    ASSERT_TRUE(checkConst(ast, secondConsts[0], 127, false, false, false));
    ASSERT_TRUE(checkConst(ast, secondConsts[1], 128, true, false, false));
    ASSERT_TRUE(checkConst(ast, secondConsts[2], uint64_t(-130), false, false, false));
    ASSERT_TRUE(checkConst(ast, secondConsts[3], 5, false, false, false));
    ASSERT_TRUE(checkConst(ast, secondConsts[4], 367, false, false, false));
}

TEST(idlc, IntegerOutOfRangeWarnAsErrors) {
    const auto [result, ast, messages] = compile("w2004.idl", true);
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0], "error [W2004]: Integer Int8 with value 128 out of range [-128, 127] at w2004:19:5");
    ASSERT_EQ(messages[1], "error [W2004]: Integer Int8 with value -130 out of range [-128, 127] at w2004:20:20");
    ASSERT_EQ(messages[2], "error [W2004]: Integer Int8 with value 367 out of range [-128, 127] at w2004:22:20");
    ASSERT_FALSE(idl_compilation_result_has_notes(ast));
    ASSERT_FALSE(idl_compilation_result_has_warnings(ast));
    ASSERT_TRUE(idl_compilation_result_has_errors(ast));

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto enums = getChilds(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_EQ(enums.size(), 2);

    auto firstConsts = getChilds(ast, enums[0], IDL_AST_NODE_TYPE_CONST);
    ASSERT_EQ(firstConsts.size(), 3);
    ASSERT_TRUE(checkConst(ast, firstConsts[0], 100, false, false, false));
    ASSERT_TRUE(checkConst(ast, firstConsts[1], 300, false, false, false));
    ASSERT_TRUE(checkConst(ast, firstConsts[2], 367, false, false, false));

    auto secondConsts = getChilds(ast, enums[1], IDL_AST_NODE_TYPE_CONST);
    ASSERT_EQ(secondConsts.size(), 5);
    ASSERT_TRUE(checkConst(ast, secondConsts[0], 127, false, false, false));
    ASSERT_TRUE(checkConst(ast, secondConsts[1], 128, true, true, false));
    ASSERT_TRUE(checkConst(ast, secondConsts[2], uint64_t(-130), false, true, false));
    ASSERT_TRUE(checkConst(ast, secondConsts[3], 5, false, false, false));
    ASSERT_TRUE(checkConst(ast, secondConsts[4], 367, false, true, false));
}

TEST(idlc, SyntaxError) {
    const auto [result, ast, messages] = compile("e3001.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3001]: Syntax error at e3001:6:5");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_EQ(api, HandleNone);
}

TEST(idlc, ArgumentParsingError) {
    const auto [result, ast, messages] = compile("e3002.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0], "error [E3002]: Argument parsing error 'invalid arg' at e3002:6:24");
    ASSERT_EQ(messages[1],
              "error [E3003]: The [version] attribute must have three required integer parameters, such as version(1, "
              "2, 3) or version(\"string\") at e3002:6:10");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto version = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_VERSION);
    ASSERT_NE(version, HandleNone);

    auto args = getChilds(ast, version);
    ASSERT_EQ(args.size(), 0);
}

TEST(idlc, VersionAttrRequiredParams) {
    const auto [result, ast, messages] = compile("e3003.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3003]: The [version] attribute must have three required integer parameters, such as version(1, "
              "2, 3) or version(\"string\") at e3003:6:10");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto version = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_VERSION);
    ASSERT_NE(version, HandleNone);

    auto args = getChilds(ast, version);
    ASSERT_EQ(args.size(), 0);
}

TEST(idlc, VersionComponentRange) {
    const auto [result, ast, messages] = compile("e3004.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0],
              "error [E3004]: Version values must be between 0 and 255, while the argument is -1 at e3004:6:10");
    ASSERT_EQ(messages[1],
              "error [E3004]: Version values must be between 0 and 255, while the argument is 256 at e3004:6:10");
    ASSERT_EQ(messages[2],
              "error [E3004]: Version values must be between 0 and 255, while the argument is 1000 at e3004:6:10");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto version = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_VERSION);
    ASSERT_NE(version, HandleNone);

    auto args = getChilds(ast, version);
    ASSERT_EQ(args.size(), 0);
}

TEST(idlc, InvalidAttrForDeclaration) {
    const auto [result, ast, messages] = compile("e3005.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0],
              "error [E3005]: Invalid attribute [flags] for api 'Api' declaration, allowed attributes are cname, "
              "tokenizer, order, single, version, brief, detail, author, copyright, license at e3005:6:10");
    ASSERT_EQ(messages[1],
              "error [E3005]: Invalid attribute [hex] for api 'Api' declaration, allowed attributes are cname, "
              "tokenizer, order, single, version, brief, detail, author, copyright, license at e3005:6:17");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto attrs = getAttrs(ast, api);
    ASSERT_EQ(attrs.size(), 7);
    ASSERT_TRUE(isType(ast, attrs[0], IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF));
    ASSERT_TRUE(isType(ast, attrs[1], IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL));
    ASSERT_TRUE(isType(ast, attrs[2], IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT));
    ASSERT_TRUE(isType(ast, attrs[3], IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE));
    ASSERT_TRUE(isType(ast, attrs[4], IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR));
    ASSERT_TRUE(isType(ast, attrs[5], IDL_AST_NODE_TYPE_ATTR_FLAGS));
    ASSERT_TRUE(isType(ast, attrs[6], IDL_AST_NODE_TYPE_ATTR_HEX));
}

TEST(idlc, AttrNotAllowedForDeclaration) {
    const auto [result, ast, messages] = compile("e3006.idl");
    deferred(idl_compilation_result_destroy(ast));
    GTEST_FAIL();
}

TEST(idlc, AttributeDuplication) {
    const auto [result, ast, messages] = compile("e3007.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3007]: Attribute duplication for attribute [version] in api 'Api' at e3007:6:28");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto attrs = getAttrs(ast, api);
    ASSERT_EQ(attrs.size(), 7);
    ASSERT_TRUE(isType(ast, attrs[0], IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF));
    ASSERT_TRUE(isType(ast, attrs[1], IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL));
    ASSERT_TRUE(isType(ast, attrs[2], IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT));
    ASSERT_TRUE(isType(ast, attrs[3], IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE));
    ASSERT_TRUE(isType(ast, attrs[4], IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR));
    ASSERT_TRUE(isType(ast, attrs[5], IDL_AST_NODE_TYPE_ATTR_VERSION));
    ASSERT_TRUE(isType(ast, attrs[6], IDL_AST_NODE_TYPE_ATTR_VERSION));
    ASSERT_NE(attrs[5], attrs[6]);
}

TEST(idlc, AttributeMustNotHaveArguments) {
    const auto [result, ast, messages] = compile("e3008.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3008]: The attribute [hex] must not have arguments at e3008:10:12");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto hex = findChild(ast, test, IDL_AST_NODE_TYPE_ATTR_HEX);
    ASSERT_NE(hex, HandleNone);

    auto hexArgs = getChilds(ast, hex);
    ASSERT_TRUE(hexArgs.empty());
}

TEST(idlc, StringClosingCharacter) {
    const auto [result, ast, messages] = compile("e3009.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(
        messages[0],
        "error [E3009]: String closing character not found in string: \"Lost closing character)])] at e3009:5:17");
    ASSERT_EQ(messages[1], "error [E3001]: Syntax error at e3009:5:45");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_EQ(api, HandleNone);
}

TEST(idlc, ApiRedeclaration) {
    const auto [result, ast, messages] = compile("e3010.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3010]: API Redeclaration 'Other' at e3010:17:1");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);
    ASSERT_EQ(getStr(ast, api), "Api");

    auto other = findChild(ast, api, IDL_AST_NODE_TYPE_API);
    ASSERT_EQ(other, HandleNone);
}

TEST(idlc, FirstDeclaration) {
    const auto [result, ast, messages] = compile("e3011.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3011]: The first declaration in the description should always begin with the 'api' declaration");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_EQ(test, HandleNone);
}

TEST(idlc, SymbolRedefinition) {
    const auto [result, ast, messages] = compile("e3012.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0], "error [E3012]: Symbol redefinition 'Api.Test.Value' at e3012:16:5");
    ASSERT_EQ(messages[1], "error [E3012]: Symbol redefinition 'Api.Test.TeSt' at e3012:17:5");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = getChilds(ast, api, IDL_AST_NODE_TYPE_ENUM).back();
    ASSERT_NE(test, HandleNone);

    auto consts = getChilds(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_EQ(consts.size(), 4);
    ASSERT_FALSE(hasAllState(ast, consts[0], IDL_AST_NODE_STATE_EVAULATED_BIT));
    ASSERT_FALSE(hasAllState(ast, consts[1], IDL_AST_NODE_STATE_EVAULATED_BIT));
    ASSERT_FALSE(hasAllState(ast, consts[2], IDL_AST_NODE_STATE_EVAULATED_BIT));
    ASSERT_FALSE(hasAllState(ast, consts[3], IDL_AST_NODE_STATE_EVAULATED_BIT));
}

TEST(idlc, UnknownAttribute) {
    const auto [result, ast, messages] = compile("e3013.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0], "error [E3013]: Unknown attribute [abcd] at e3013:6:13");
    ASSERT_EQ(messages[1], "error [E3013]: Unknown attribute [invalidattr] at e3013:7:31");
    ASSERT_EQ(messages[2], "error [E3013]: Unknown attribute [xyz] at e3013:7:69");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto attrs = getAttrs(ast, api);
    ASSERT_EQ(attrs.size(), 6);
    ASSERT_TRUE(isType(ast, attrs[0], IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF));
    ASSERT_TRUE(isType(ast, attrs[1], IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL));
    ASSERT_TRUE(isType(ast, attrs[2], IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT));
    ASSERT_TRUE(isType(ast, attrs[3], IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE));
    ASSERT_TRUE(isType(ast, attrs[4], IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR));
    ASSERT_TRUE(isType(ast, attrs[5], IDL_AST_NODE_TYPE_ATTR_VERSION));
}

TEST(idlc, BriefAttrMustContainOneOrMoreArgs) {
    const auto [result, ast, messages] = compile("e3014.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3014]: The [brief] attribute must contain one or more arguments at e3014:5:10");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto attrs = getAttrs(ast, api);
    ASSERT_EQ(attrs.size(), 5);
    ASSERT_TRUE(isType(ast, attrs[0], IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL));
    ASSERT_TRUE(isType(ast, attrs[1], IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT));
    ASSERT_TRUE(isType(ast, attrs[2], IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE));
    ASSERT_TRUE(isType(ast, attrs[3], IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR));
    ASSERT_TRUE(isType(ast, attrs[4], IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF));

    auto args = getChilds(ast, attrs[4]);
    ASSERT_TRUE(args.empty());
}

TEST(idlc, UnknownAttributeInDoc) {
    const auto [result, ast, messages] = compile("e3015.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
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

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto attrs = getAttrs(ast, api);
    ASSERT_EQ(attrs.size(), 7);
    ASSERT_TRUE(isType(ast, attrs[0], IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF));
    ASSERT_TRUE(isType(ast, attrs[1], IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL));
    ASSERT_TRUE(isType(ast, attrs[2], IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT));
    ASSERT_TRUE(isType(ast, attrs[3], IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE));
    ASSERT_TRUE(isType(ast, attrs[4], IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR));
    ASSERT_TRUE(isType(ast, attrs[5], IDL_AST_NODE_TYPE_ATTR_FLAGS));
    ASSERT_TRUE(isType(ast, attrs[6], IDL_AST_NODE_TYPE_ATTR_HEX));

    auto briefArgs = getChilds(ast, attrs[0]);
    ASSERT_FALSE(briefArgs.empty());

    auto flagsArgs = getChilds(ast, attrs[5]);
    ASSERT_TRUE(flagsArgs.empty());

    auto hexArgs = getChilds(ast, attrs[6]);
    ASSERT_TRUE(hexArgs.empty());
}

TEST(idlc, DocumentationStringEmpty) {
    const auto [result, ast, messages] = compile("e3016");
    deferred(idl_compilation_result_destroy(ast));
    GTEST_FAIL();
}

TEST(idlc, DetailAttrMustContainOneOrMoreArgs) {
    const auto [result, ast, messages] = compile("e3017");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3017]: The [detail] attribute must contain one or more arguments at e3017:5:10");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto attrs = getAttrs(ast, api);
    ASSERT_EQ(attrs.size(), 5);
    ASSERT_TRUE(isType(ast, attrs[0], IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF));
    ASSERT_TRUE(isType(ast, attrs[1], IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT));
    ASSERT_TRUE(isType(ast, attrs[2], IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE));
    ASSERT_TRUE(isType(ast, attrs[3], IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR));
    ASSERT_TRUE(isType(ast, attrs[4], IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL));

    auto args = getChilds(ast, attrs[4]);
    ASSERT_TRUE(args.empty());
}

TEST(idlc, InlineDocAllowedDetailOnlyAttr) {
    const auto [result, ast, messages] = compile("e3018");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3018]: Inline documentation only [detail] description is allowed at e3018:5:37");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto attrs = getAttrs(ast, api);
    ASSERT_EQ(attrs.size(), 5);
    ASSERT_TRUE(isType(ast, attrs[0], IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL));
    ASSERT_TRUE(isType(ast, attrs[1], IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT));
    ASSERT_TRUE(isType(ast, attrs[2], IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE));
    ASSERT_TRUE(isType(ast, attrs[3], IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR));
    ASSERT_TRUE(isType(ast, attrs[4], IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF));
}

TEST(idlc, OrderAttrCanContainOneOptionalBoolParam) {
    const auto [result, ast, messages] = compile("e3019");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0],
              "error [E3019]: The [order] attribute can contain one optional Boolean parameter at e3019:6:10");
    ASSERT_EQ(messages[1],
              "error [E3019]: The [order] attribute can contain one optional Boolean parameter at e3019:6:22");
    ASSERT_EQ(messages[2], "error [E3007]: Attribute duplication for attribute [order] in api 'Api' at e3019:6:22");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto attrs = getAttrs(ast, api, AttrFilterNonDoc);
    ASSERT_EQ(attrs.size(), 2);
    ASSERT_TRUE(getChilds(ast, attrs[0]).empty());
    ASSERT_TRUE(getChilds(ast, attrs[1]).empty());
}

TEST(idlc, TabsNotAllowed) {
    const auto [result, ast, messages] = compile("e3020");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3020]: Tabs are not allowed at e3020:6:4");
}

TEST(idlc, CouldNotFindFileForImport) {
    const auto [result, ast, messages] = compile("e3021");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3021]: could not find file 'NotFoundImport' for import at e3021:9:1");

    const auto [result2, ast2, messages2] = compile("e3021nonexists");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result2, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages2.size(), 1);
    ASSERT_EQ(messages2[0], "error [E3021]: could not find file 'e3021nonexists' for import");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto notFoundImport = findChild(ast, api, IDL_AST_NODE_TYPE_IMPORT);
    ASSERT_NE(notFoundImport, HandleNone);

    auto importAttrs = getChilds(ast, notFoundImport);
    ASSERT_EQ(importAttrs.size(), 2);
    ASSERT_TRUE(isType(ast, importAttrs[0], IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF));
    ASSERT_TRUE(isType(ast, importAttrs[1], IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL));

    ASSERT_EQ(idl_compilation_result_get_api(ast2), HandleNone);
}

TEST(idlc, ConstCanBeDefinedOnlyForEnum) {
    const auto [result, ast, messages] = compile("e3023");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3023]: A 'const' of 'Value' can be defined only for an 'enum' at e3023:7:5");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto value = findChild(ast, api, IDL_AST_NODE_TYPE_CONST);
    ASSERT_EQ(value, HandleNone);
}

TEST(idlc, ValueAttrMustContainOneOrMoreArgs) {
    const auto [result, ast, messages] = compile("e3024");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3024]: The [value] attribute must contain one or more arguments at e3024:10:18");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto value = findChild(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_NE(value, HandleNone);

    auto attrValue = findChild(ast, value, IDL_AST_NODE_TYPE_ATTR_VALUE);
    ASSERT_NE(attrValue, HandleNone);

    auto attrValueArgs = getChilds(ast, attrValue);
    ASSERT_TRUE(attrValueArgs.empty());
}

TEST(idlc, ValueAttrArgsMustBeLiteralsOrDeclReference) {
    const auto [result, ast, messages] = compile("e3025");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0], "error [E3002]: Argument parsing error 'fail' at e3025:10:24");
    ASSERT_EQ(messages[1], "error [E3025]: Arguments for the [value] attribute must be literals at e3025:10:18");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto value = findChild(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_NE(value, HandleNone);

    auto attrValue = findChild(ast, value, IDL_AST_NODE_TYPE_ATTR_VALUE);
    ASSERT_NE(attrValue, HandleNone);

    auto attrValueArgs = getChilds(ast, attrValue);
    ASSERT_TRUE(attrValueArgs.empty());
}

TEST(idlc, AllLiteralsInTheValueAttrMustBeOfSameType) {
    const auto [result, ast, messages] = compile("e3026");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3026]: All literals in the [value] attribute must be of the same type at e3026:10:18");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto value = findChild(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_NE(value, HandleNone);

    auto attrValue = findChild(ast, value, IDL_AST_NODE_TYPE_ATTR_VALUE);
    ASSERT_NE(attrValue, HandleNone);

    auto attrValueArgs = getChilds(ast, attrValue);
    ASSERT_TRUE(attrValueArgs.empty());
}

TEST(idlc, TypeAttrArgCanOnlyReferToSymbols) {
    const auto [result, ast, messages] = compile("e3027");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3027]: The [type] attribute argument can only refer to symbols at e3027:9:12");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto attrType = findChild(ast, test, IDL_AST_NODE_TYPE_ATTR_TYPE);
    ASSERT_NE(attrType, HandleNone);

    auto declRef = findChild(ast, attrType, IDL_AST_NODE_TYPE_DECL_REF);
    ASSERT_EQ(declRef, HandleNone);
}

TEST(idlc, CnameAttrMustContainSingleStringLiteralArg) {
    const auto [result, ast, messages] = compile("e3028");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0], "error [E3002]: Argument parsing error 'not string' at e3028:6:16");
    ASSERT_EQ(messages[1],
              "error [E3028]: The [cname] attribute must contain a single string literal argument at e3028:6:10");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto cname = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_CNAME);
    ASSERT_NE(cname, HandleNone);

    auto cnameArgs = getChilds(ast, cname);
    ASSERT_TRUE(cnameArgs.empty());
}

TEST(idlc, CnameAttrMustSpecifyNameWithoutSpacesAndPuncts) {
    const auto [result, ast, messages] = compile("e3029");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
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

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto consts = getChilds(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_EQ(consts.size(), 2);

    for (auto c : consts) {
        auto value     = findChild(ast, c, IDL_AST_NODE_TYPE_ATTR_CNAME);
        auto valueArgs = getChilds(ast, value);
        ASSERT_TRUE(valueArgs.empty());
    }
}

TEST(idlc, SingleAttrCanContainOneOptionalBoolParam) {
    const auto [result, ast, messages] = compile("e3030");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0],
              "error [E3030]: The [single] attribute can contain one optional Boolean parameter at e3030:6:10");
    ASSERT_EQ(messages[1],
              "error [E3030]: The [single] attribute can contain one optional Boolean parameter at e3030:6:23");
    ASSERT_EQ(messages[2], "error [E3007]: Attribute duplication for attribute [single] in api 'Api' at e3030:6:23");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto attrs = getAttrs(ast, api, AttrFilterNonDoc);
    for (auto attr : attrs) {
        auto attrArgs = getChilds(ast, attr);
        ASSERT_TRUE(attrArgs.empty());
    }
}

TEST(idlc, InvalidTokenizerFormatString) {
    const auto [result, ast, messages] = compile("e3031");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(
        messages[0],
        "error [E3031]: Invalid tokenizer format string \"2-a3-4\", a valid string looks like (2-^3-4) at e3031:6:10");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto attrs = getAttrs(ast, api, AttrFilterNonDoc);
    for (auto attr : attrs) {
        auto attrArgs = getChilds(ast, attr);
        ASSERT_TRUE(attrArgs.empty());
    }
}

TEST(idlc, IntTokenizationParamsOrFmtStringMustBePassedToTokenizerAttr) {
    const auto [result, ast, messages] = compile("e3032");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0],
              "error [E3032]: Integer tokenization parameters or a tokenizer string must be passed to the attribute "
              "[tokenizer] at e3032:6:10");
    ASSERT_EQ(messages[1],
              "error [E3032]: Integer tokenization parameters or a tokenizer string must be passed to the attribute "
              "[tokenizer] at e3032:10:18");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto attrs = getAttrs(ast, api, AttrFilterNonDoc);
    for (auto attr : attrs) {
        auto attrArgs = getChilds(ast, attr);
        ASSERT_TRUE(attrArgs.empty());
    }
}

TEST(idlc, TokenizerAttrMustContainOneOrMoreArgs) {
    const auto [result, ast, messages] = compile("e3033");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3033]: The [tokenizer] attribute must contain one or more arguments (integers: 2, -2, 4 or "
              "string \"2-^3-4\") at e3033:6:10");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto attrs = getAttrs(ast, api, AttrFilterNonDoc);
    for (auto attr : attrs) {
        auto attrArgs = getChilds(ast, attr);
        ASSERT_TRUE(attrArgs.empty());
    }
}

TEST(idlc, CopyrightAttrMustContainOneOrMoreArgs) {
    const auto [result, ast, messages] = compile("e3034");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3034]: The [copyright] attribute must contain one or more arguments at e3034:5:10");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto copyright = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT);
    ASSERT_NE(copyright, HandleNone);

    auto copyrightArgs = getChilds(ast, copyright);
    ASSERT_TRUE(copyrightArgs.empty());
}

TEST(idlc, LicenseAttrMustContainOneOrMoreArgs) {
    const auto [result, ast, messages] = compile("e3035");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3035]: The [license] attribute must contain one or more arguments at e3035:5:10");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto license = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE);
    ASSERT_NE(license, HandleNone);

    auto licenseArgs = getChilds(ast, license);
    ASSERT_TRUE(licenseArgs.empty());
}

TEST(idlc, IdentifiersCaseSensitive) {
    const auto [result, ast, messages] = compile("e3036");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0],
              "error [E3036]: Identifiers are case sensitive, error in 'ApI', but expected 'Api' at e3036:1:10");
    ASSERT_EQ(messages[1],
              "error [E3036]: Identifiers are case sensitive, error in 'Api.Test.value', but expected 'Api.Test.Value' "
              "at e3036:2:22");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto brief = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF);
    ASSERT_NE(brief, HandleNone);

    auto declRef = findChild(ast, brief, IDL_AST_NODE_TYPE_DECL_REF);
    ASSERT_NE(brief, HandleNone);

    auto symbol = getDeclRef(ast, declRef);
    ASSERT_EQ(symbol, HandleNone);
}

TEST(idlc, SymbolDefinitionNotFound) {
    const auto [result, ast, messages] = compile("e3037");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3037]: Symbol definition 'Test' not found at e3037:1:10");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto brief = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF);
    ASSERT_NE(brief, HandleNone);

    auto declRef = findChild(ast, brief, IDL_AST_NODE_TYPE_DECL_REF);
    ASSERT_NE(brief, HandleNone);

    auto symbol = getDeclRef(ast, declRef);
    ASSERT_EQ(symbol, HandleNone);
}

TEST(idlc, ConstCanOnlyReferToOtherConstWhenEvaluated) {
    const auto [result, ast, messages] = compile("e3038");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3038]: Constants can only refer to other constants when evaluated at e3038:10:5");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto consts = getChilds(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_EQ(consts.size(), 2);
    ASSERT_TRUE(checkConst(ast, consts[0], 0, false, true, false));
    ASSERT_TRUE(checkConst(ast, consts[1], 1, false, false, false));
}

TEST(idlc, ConstCannotReferToItselfWhenEvaluated) {
    const auto [result, ast, messages] = compile("e3039");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3039]: A constant 'Api.Test.Value' cannot refer to itself when evaluated at e3039:10:5");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto consts = getChilds(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_EQ(consts.size(), 1);
    ASSERT_TRUE(checkConst(ast, consts.front(), 0, false, true, false));
}

TEST(idlc, EnumConstsCanOnlyBeSpecifiedAsIntOrEnumConsts) {
    const auto [result, ast, messages] = compile("e3040");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0],
              "error [E3040]: Enumeration constants 'Api.Test.Value1' can only be specified as integers or enum consts "
              "at e3040:10:20");
    ASSERT_EQ(messages[1],
              "error [E3040]: Enumeration constants 'Api.Test.Value2' can only be specified as integers or enum consts "
              "at e3040:11:20");
    ASSERT_EQ(messages[2],
              "error [E3040]: Enumeration constants 'Api.Test.Value3' can only be specified as integers or enum consts "
              "at e3040:12:20");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto consts = getChilds(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_EQ(consts.size(), 3);
    ASSERT_TRUE(checkConst(ast, consts[0], 0, false, true, false));
    ASSERT_TRUE(checkConst(ast, consts[1], 0, false, true, false));
    ASSERT_TRUE(checkConst(ast, consts[2], 0, false, true, false));
}

TEST(idlc, FailedCalculateConst) {
    const auto [result, ast, messages] = compile("e3041");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0],
              "error [E3040]: Enumeration constants 'Api.Test.Value1' can only be specified as integers or enum consts "
              "at e3041:10:20");
    ASSERT_EQ(messages[1], "error [E3041]: Failed to calculate the constant 'Api.Test.Value2' at e3041:11:5");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto consts = getChilds(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_EQ(consts.size(), 4);
    ASSERT_TRUE(checkConst(ast, consts[0], 0, false, true, false));
    ASSERT_TRUE(checkConst(ast, consts[1], 0, false, true, false));
    ASSERT_TRUE(checkConst(ast, consts[2], 0, false, true, false));
    ASSERT_TRUE(checkConst(ast, consts[3], 0, true, true, false));
}

TEST(idlc, CyclicDependenceOfConst) {
    const auto [result, ast, messages] = compile("e3042");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 6);
    ASSERT_EQ(messages[0],
              "warning [W2003]: The constant 'Api.Test.Value1' refers to a constant declared below 'Api.Test.Value5' "
              "at e3042:10:5");
    ASSERT_EQ(messages[1],
              "error [E3042]: Cyclic dependence of constant 'Api.Test.Value1 -> Api.Test.Value5 -> Api.Test.Value4 -> "
              "Api.Test.Value3 -> Api.Test.Value1' at e3042:10:5");
    ASSERT_EQ(messages[2], "error [E3041]: Failed to calculate the constant 'Api.Test.Value3' at e3042:12:5");
    ASSERT_EQ(messages[3],
              "warning [W2003]: The constant 'Api.OtherTest.Value3' refers to a constant declared below "
              "'Api.OtherTest.Value8' at e3042:20:5");
    ASSERT_EQ(
        messages[4],
        "error [E3042]: Cyclic dependence of constant 'Api.OtherTest.Value3 -> Api.OtherTest.Value8 -> "
        "Api.OtherTest.Value7 -> Api.OtherTest.Value6 -> Api.OtherTest.Value4 -> Api.OtherTest.Value3' at e3042:20:5");
    ASSERT_EQ(messages[5], "error [E3041]: Failed to calculate the constant 'Api.OtherTest.Value4' at e3042:21:5");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto enums = getChilds(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_EQ(enums.size(), 2);
    auto test      = enums[0];
    auto otherTest = enums[1];

    auto testConsts = getChilds(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_EQ(testConsts.size(), 5);
    ASSERT_TRUE(checkConst(ast, testConsts[0], 0, false, true, true));
    ASSERT_TRUE(checkConst(ast, testConsts[1], 2, false, false, false));
    ASSERT_TRUE(checkConst(ast, testConsts[2], 0, false, true, false));
    ASSERT_TRUE(checkConst(ast, testConsts[3], 2, false, true, false));
    ASSERT_TRUE(checkConst(ast, testConsts[4], 0, false, true, false));

    auto otherTestConsts = getChilds(ast, otherTest, IDL_AST_NODE_TYPE_CONST);
    ASSERT_EQ(otherTestConsts.size(), 8);
    ASSERT_TRUE(checkConst(ast, otherTestConsts[0], 0, true, false, false));
    ASSERT_TRUE(checkConst(ast, otherTestConsts[1], 2, false, false, false));
    ASSERT_TRUE(checkConst(ast, otherTestConsts[2], 0, false, true, true));
    ASSERT_TRUE(checkConst(ast, otherTestConsts[3], 0, true, true, false));
    ASSERT_TRUE(checkConst(ast, otherTestConsts[4], 0, true, true, false));
    ASSERT_TRUE(checkConst(ast, otherTestConsts[5], 0, false, true, false));
    ASSERT_TRUE(checkConst(ast, otherTestConsts[6], 0, true, true, false));
    ASSERT_TRUE(checkConst(ast, otherTestConsts[7], 0, true, true, false));
}

TEST(idlc, TypeAttrMustContainOnlyOneType) {
    const auto [result, ast, messages] = compile("e3043");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3043]: The [type] attribute must contain only one type at e3043:9:12");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto attrType = findChild(ast, test, IDL_AST_NODE_TYPE_ATTR_TYPE);
    ASSERT_NE(test, HandleNone);

    auto declRef = findChild(ast, attrType, IDL_AST_NODE_TYPE_DECL_REF);
    ASSERT_NE(declRef, HandleNone);

    auto decl = getDeclRef(ast, declRef);
    ASSERT_EQ(decl, HandleNone);
}

TEST(idlc, EnumCanOnlyOfIntsType) {
    const auto [result, ast, messages] = compile("e3044");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3044]: Enumeration 'Api.Test' can only of integers type at e3044:9:1");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto attrType = findChild(ast, test, IDL_AST_NODE_TYPE_ATTR_TYPE);
    ASSERT_NE(test, HandleNone);

    auto declRef = findChild(ast, attrType, IDL_AST_NODE_TYPE_DECL_REF);
    ASSERT_NE(declRef, HandleNone);

    auto type = getDeclRef(ast, declRef);
    ASSERT_NE(type, HandleNone);
    ASSERT_TRUE(isType(ast, type, IDL_AST_NODE_TYPE_FLOAT_TYPE));
    ASSERT_FALSE(isType(ast, type, IDL_AST_NODE_TYPE_INTEGER_TYPE));
}

TEST(idlc, EnumMustContainAtLeastOneConst) {
    const auto [result, ast, messages] = compile("e3045");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3045]: Enumeration 'Api.Test' must contain at least one constant at e3045:9:1");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto consts = getChilds(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_TRUE(consts.empty());
}

TEST(idlc, ResultCode) {
    const auto [result, _, __] = compile("e3021nonexists", false, false);
    ASSERT_EQ(result, IDL_RESULT_ERROR_SOURCE_NOT_FOUND);

    const auto [result2, _2, __2] = compile("e3045", false, false);
    ASSERT_EQ(result2, IDL_RESULT_ERROR_COMPILATION);

    const auto [result3, _3, __3] = compile("w2002.idl", false, false);
    ASSERT_EQ(result3, IDL_RESULT_SUCCESS);

    const auto [result4, _4, __4] = compile("w2002.idl", true, false);
    ASSERT_EQ(result4, IDL_RESULT_ERROR_COMPILATION);
}
