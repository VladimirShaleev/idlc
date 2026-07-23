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

TEST(idlc, FloatOutOfRange) {
    const auto [result, ast, messages] = compile("w2004float.idl", true);
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0],
              "error [W2004]: Float32 with value 3.50282e+38 out of range [-3.4028235e+38, 3.4028235e+38] at "
              "w2004float:12:30");
    ASSERT_EQ(messages[1],
              "error [W2004]: Float32 with value -3.70282e+38 out of range [-3.4028235e+38, 3.4028235e+38] at "
              "w2004float:13:30");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(test, HandleNone);
    ASSERT_TRUE(hasAllState(ast, test, IDL_AST_NODE_STATE_EVAULATED_BIT));

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_EQ(fields.size(), 3);
    ASSERT_TRUE(hasAllState(ast, fields[0], IDL_AST_NODE_STATE_EVAULATED_BIT));
    ASSERT_TRUE(hasAllState(ast, fields[1], IDL_AST_NODE_STATE_EVAULATED_BIT | IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_TRUE(hasAllState(ast, fields[2], IDL_AST_NODE_STATE_EVAULATED_BIT | IDL_AST_NODE_STATE_BUILD_ERROR_BIT));

    auto attrValue0 = findChild(ast, fields[0], IDL_AST_NODE_TYPE_ATTR_VALUE);
    auto attrValue1 = findChild(ast, fields[1], IDL_AST_NODE_TYPE_ATTR_VALUE);
    auto attrValue2 = findChild(ast, fields[2], IDL_AST_NODE_TYPE_ATTR_VALUE);
    ASSERT_NE(attrValue0, HandleNone);
    ASSERT_NE(attrValue1, HandleNone);
    ASSERT_NE(attrValue1, HandleNone);

    auto value0 = findChild(ast, attrValue0, IDL_AST_NODE_TYPE_LITERAL_FLOAT);
    auto value1 = findChild(ast, attrValue1, IDL_AST_NODE_TYPE_LITERAL_FLOAT);
    auto value2 = findChild(ast, attrValue2, IDL_AST_NODE_TYPE_LITERAL_FLOAT);
    ASSERT_NE(value0, HandleNone);
    ASSERT_NE(value1, HandleNone);
    ASSERT_NE(value2, HandleNone);

    ASSERT_DOUBLE_EQ(getFloat(ast, value0), 32.73);
    ASSERT_DOUBLE_EQ(getFloat(ast, value1), 3.5028235e+38);
    ASSERT_DOUBLE_EQ(getFloat(ast, value2), -3.7028235e+38);
}

TEST(idlc, IntegerOutOfRange) {
    const auto [result, ast, messages] = compile("w2004int.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0], "warning [W2004]: Int8 with value 128 out of range [-128, 127] at w2004int:19:5");
    ASSERT_EQ(messages[1], "warning [W2004]: Int8 with value -130 out of range [-128, 127] at w2004int:20:20");
    ASSERT_EQ(messages[2], "warning [W2004]: Int8 with value 367 out of range [-128, 127] at w2004int:22:20");
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
    ASSERT_TRUE(checkConst(ast, secondConsts[2], -130, false, false, false));
    ASSERT_TRUE(checkConst(ast, secondConsts[3], 5, false, false, false));
    ASSERT_TRUE(checkConst(ast, secondConsts[4], 367, false, false, false));
}

TEST(idlc, IntegerOutOfRangeWarnAsErrors) {
    const auto [result, ast, messages] = compile("w2004int.idl", true);
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0], "error [W2004]: Int8 with value 128 out of range [-128, 127] at w2004int:19:5");
    ASSERT_EQ(messages[1], "error [W2004]: Int8 with value -130 out of range [-128, 127] at w2004int:20:20");
    ASSERT_EQ(messages[2], "error [W2004]: Int8 with value 367 out of range [-128, 127] at w2004int:22:20");
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
    ASSERT_TRUE(checkConst(ast, secondConsts[2], -130, false, true, false));
    ASSERT_TRUE(checkConst(ast, secondConsts[3], 5, false, false, false));
    ASSERT_TRUE(checkConst(ast, secondConsts[4], 367, false, true, false));
}

TEST(idlc, SpecialCharacterExpectedAfterBackslash) {
    const auto [result, ast, messages] = compile("w2005.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 6);
    ASSERT_EQ(messages[0],
              "warning [W2005]: Special character expected after backslash; valid spacial characters '\\\\', '\\{', "
              "'\\}', '\\[', '\\]', '\\`' at w2005:2:13");
    ASSERT_EQ(messages[1],
              "warning [W2005]: Special character expected after backslash; valid spacial characters '\\\\', '\\{', "
              "'\\}', '\\[', '\\]', '\\`' at w2005:2:16");
    ASSERT_EQ(messages[2],
              "warning [W2005]: Special character expected after backslash; valid spacial characters '\\\\', '\\{', "
              "'\\}', '\\[', '\\]', '\\`' at w2005:4:30");
    ASSERT_EQ(messages[3],
              "warning [W2005]: Special character expected after backslash; valid spacial characters '\\\\', '\\{', "
              "'\\}', '\\[', '\\]', '\\s', '\\t', '\\n' at w2005:6:3");
    ASSERT_EQ(messages[4],
              "warning [W2005]: Special character expected after backslash; valid spacial characters '\\\\', '\\{', "
              "'\\}', '\\[', '\\]', '\\s', '\\t', '\\n' at w2005:6:19");
    ASSERT_EQ(messages[5],
              "warning [W2005]: Special character expected after backslash; valid spacial characters '\\\\', '\\{', "
              "'\\}', '\\[', '\\]', '\\s', '\\t', '\\n' at w2005:6:49");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto consts = getChilds(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_EQ(consts.size(), 3);

    auto value1 = consts[0];
    auto value2 = consts[1];
    auto value3 = consts[2];
    ASSERT_NE(value1, HandleNone);
    ASSERT_NE(value2, HandleNone);
    ASSERT_NE(value3, HandleNone);

    auto apiBrief     = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF);
    auto apiDetail    = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL);
    auto apiCopyright = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT);
    auto apiLicense   = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE);
    auto apiAuthor    = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR);
    auto testBrief    = findChild(ast, test, IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF);
    auto testDetail   = findChild(ast, test, IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL);
    auto value1Detail = findChild(ast, value1, IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL);
    auto value2Detail = findChild(ast, value2, IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL);
    auto value3Detail = findChild(ast, value3, IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL);
    ASSERT_NE(apiBrief, HandleNone);
    ASSERT_NE(apiDetail, HandleNone);
    ASSERT_NE(apiCopyright, HandleNone);
    ASSERT_NE(apiLicense, HandleNone);
    ASSERT_NE(apiAuthor, HandleNone);
    ASSERT_NE(testBrief, HandleNone);
    ASSERT_NE(testDetail, HandleNone);
    ASSERT_NE(value1Detail, HandleNone);
    ASSERT_NE(value2Detail, HandleNone);
    ASSERT_NE(value3Detail, HandleNone);
    ASSERT_TRUE(hasAllState(ast, apiBrief, IDL_AST_NODE_STATE_MULTILINE_DOC_BIT));
    ASSERT_FALSE(hasAllState(ast, apiDetail, IDL_AST_NODE_STATE_MULTILINE_DOC_BIT));
    ASSERT_FALSE(hasAllState(ast, apiCopyright, IDL_AST_NODE_STATE_MULTILINE_DOC_BIT));
    ASSERT_FALSE(hasAllState(ast, apiLicense, IDL_AST_NODE_STATE_MULTILINE_DOC_BIT));
    ASSERT_TRUE(hasAllState(ast, apiAuthor, IDL_AST_NODE_STATE_MULTILINE_DOC_BIT));
    ASSERT_FALSE(hasAllState(ast, testBrief, IDL_AST_NODE_STATE_MULTILINE_DOC_BIT));
    ASSERT_FALSE(hasAllState(ast, testDetail, IDL_AST_NODE_STATE_MULTILINE_DOC_BIT));
    ASSERT_FALSE(hasAllState(ast, value1Detail, IDL_AST_NODE_STATE_MULTILINE_DOC_BIT));
    ASSERT_TRUE(hasAllState(ast, value2Detail, IDL_AST_NODE_STATE_MULTILINE_DOC_BIT));
    ASSERT_FALSE(hasAllState(ast, value3Detail, IDL_AST_NODE_STATE_MULTILINE_DOC_BIT));

    auto apiBriefArgs = getChilds(ast, apiBrief);
    ASSERT_EQ(apiBriefArgs.size(), 25);
    ASSERT_EQ(getStr(ast, apiBriefArgs[0]), "Brief");
    ASSERT_EQ(getStr(ast, apiBriefArgs[1]), "   ");
    ASSERT_EQ(getStr(ast, apiBriefArgs[2]), "\\");
    ASSERT_EQ(getStr(ast, apiBriefArgs[3]), "  ");
    ASSERT_EQ(getStr(ast, apiBriefArgs[4]), "\\");
    ASSERT_EQ(getStr(ast, apiBriefArgs[5]), "atest");
    ASSERT_EQ(getStr(ast, apiBriefArgs[6]), " ");
    ASSERT_EQ(getDeclRef(ast, apiBriefArgs[7]), api);
    ASSERT_EQ(getStr(ast, apiBriefArgs[8]), "\n");
    ASSERT_EQ(getStr(ast, apiBriefArgs[9]), "   ");
    ASSERT_EQ(getStr(ast, apiBriefArgs[10]), "te\\st");
    ASSERT_EQ(getStr(ast, apiBriefArgs[11]), " ");
    ASSERT_EQ(getStr(ast, apiBriefArgs[12]), "ind`ent");
    ASSERT_EQ(getStr(ast, apiBriefArgs[13]), "\n");
    ASSERT_EQ(getStr(ast, apiBriefArgs[14]), "   ");
    ASSERT_EQ(getStr(ast, apiBriefArgs[15]), "test");
    ASSERT_EQ(getStr(ast, apiBriefArgs[16]), "\t");
    ASSERT_EQ(getStr(ast, apiBriefArgs[17]), "{A}");
    ASSERT_EQ(getStr(ast, apiBriefArgs[18]), " ");
    ASSERT_EQ(getStr(ast, apiBriefArgs[19]), "Str[B]123");
    ASSERT_EQ(getStr(ast, apiBriefArgs[20]), "\\");
    ASSERT_EQ(getStr(ast, apiBriefArgs[21]), "n");
    ASSERT_EQ(getStr(ast, apiBriefArgs[22]), "\n");
    ASSERT_EQ(getStr(ast, apiBriefArgs[23]), "   ");
    ASSERT_EQ(getStr(ast, apiBriefArgs[24]), "OK`");

    auto apiDetailArgs = getChilds(ast, apiDetail);
    ASSERT_EQ(apiDetailArgs.size(), 17);
    ASSERT_EQ(getStr(ast, apiDetailArgs[0]), "\\");
    ASSERT_EQ(getStr(ast, apiDetailArgs[1]), "eDetail");
    ASSERT_EQ(getStr(ast, apiDetailArgs[2]), "\n");
    ASSERT_EQ(getStr(ast, apiDetailArgs[3]), "\n");
    ASSERT_EQ(getStr(ast, apiDetailArgs[4]), "Test");
    ASSERT_EQ(getStr(ast, apiDetailArgs[5]), "\\");
    ASSERT_EQ(getStr(ast, apiDetailArgs[6]), "OK");
    ASSERT_EQ(getStr(ast, apiDetailArgs[7]), "   ");
    ASSERT_EQ(getStr(ast, apiDetailArgs[8]), " ");
    ASSERT_EQ(getStr(ast, apiDetailArgs[9]), "123");
    ASSERT_EQ(getStr(ast, apiDetailArgs[10]), " ");
    ASSERT_EQ(getStr(ast, apiDetailArgs[11]), "  ");
    ASSERT_EQ(getStr(ast, apiDetailArgs[12]), " ");
    ASSERT_EQ(getStr(ast, apiDetailArgs[13]), "AAA");
    ASSERT_EQ(getStr(ast, apiDetailArgs[14]), "\t");
    ASSERT_EQ(getStr(ast, apiDetailArgs[15]), "BBB");
    ASSERT_EQ(getStr(ast, apiDetailArgs[16]), "\\");

    auto apiCopyrightArgs = getChilds(ast, apiCopyright);
    ASSERT_EQ(apiCopyrightArgs.size(), 1);
    ASSERT_EQ(getStr(ast, apiCopyrightArgs[0]), "Copyright");

    auto apiLicenseArgs = getChilds(ast, apiLicense);
    ASSERT_EQ(apiLicenseArgs.size(), 1);
    ASSERT_EQ(getStr(ast, apiLicenseArgs[0]), "License");

    auto apiAuthorArgs = getChilds(ast, apiAuthor);
    ASSERT_EQ(apiAuthorArgs.size(), 5);
    ASSERT_EQ(getStr(ast, apiAuthorArgs[0]), "  ");
    ASSERT_EQ(getStr(ast, apiAuthorArgs[1]), "Author");
    ASSERT_EQ(getStr(ast, apiAuthorArgs[2]), "\n");
    ASSERT_EQ(getStr(ast, apiAuthorArgs[3]), "Indent");
    ASSERT_EQ(getStr(ast, apiAuthorArgs[4]), "\n");

    auto testBriefArgs = getChilds(ast, testBrief);
    ASSERT_EQ(testBriefArgs.size(), 1);
    ASSERT_EQ(getStr(ast, testBriefArgs[0]), "Brief");

    auto testDetailArgs = getChilds(ast, testDetail);
    ASSERT_EQ(testDetailArgs.size(), 1);
    ASSERT_EQ(getStr(ast, testDetailArgs[0]), "Detail");

    auto value1DetailArgs = getChilds(ast, value1Detail);
    ASSERT_EQ(value1DetailArgs.size(), 7);
    ASSERT_EQ(getStr(ast, value1DetailArgs[0]), "Test");
    ASSERT_EQ(getStr(ast, value1DetailArgs[1]), " ");
    ASSERT_EQ(getStr(ast, value1DetailArgs[2]), "first");
    ASSERT_EQ(getStr(ast, value1DetailArgs[3]), " ");
    ASSERT_EQ(getStr(ast, value1DetailArgs[4]), "doc");
    ASSERT_EQ(getStr(ast, value1DetailArgs[5]), " ");
    ASSERT_EQ(getStr(ast, value1DetailArgs[6]), "string");

    auto value2DetailArgs = getChilds(ast, value2Detail);
    ASSERT_EQ(value2DetailArgs.size(), 22);
    ASSERT_EQ(getStr(ast, value2DetailArgs[0]), "Test");
    ASSERT_EQ(getStr(ast, value2DetailArgs[1]), " ");
    ASSERT_EQ(getStr(ast, value2DetailArgs[2]), "multiline");
    ASSERT_EQ(getStr(ast, value2DetailArgs[3]), "\n");
    ASSERT_EQ(getStr(ast, value2DetailArgs[4]), "inline");
    ASSERT_EQ(getStr(ast, value2DetailArgs[5]), " ");
    ASSERT_EQ(getStr(ast, value2DetailArgs[6]), "doc");
    ASSERT_EQ(getStr(ast, value2DetailArgs[7]), " ");
    ASSERT_EQ(getStr(ast, value2DetailArgs[8]), "for");
    ASSERT_EQ(getStr(ast, value2DetailArgs[9]), " ");
    ASSERT_EQ(getStr(ast, value2DetailArgs[10]), "enum");
    ASSERT_EQ(getStr(ast, value2DetailArgs[11]), " ");
    ASSERT_EQ(getDeclRef(ast, value2DetailArgs[12]), test);
    ASSERT_EQ(getStr(ast, value2DetailArgs[13]), "\n");
    ASSERT_EQ(getStr(ast, value2DetailArgs[14]), "  ");
    ASSERT_EQ(getStr(ast, value2DetailArgs[15]), "of");
    ASSERT_EQ(getStr(ast, value2DetailArgs[16]), " ");
    ASSERT_EQ(getStr(ast, value2DetailArgs[17]), "const");
    ASSERT_EQ(getStr(ast, value2DetailArgs[18]), " ");
    ASSERT_EQ(getDeclRef(ast, value2DetailArgs[19]), value2);
    ASSERT_EQ(getStr(ast, value2DetailArgs[20]), ".");
    ASSERT_EQ(getStr(ast, value2DetailArgs[21]), "\n");

    auto value3DetailArgs = getChilds(ast, value3Detail);
    ASSERT_EQ(value3DetailArgs.size(), 3);
    ASSERT_EQ(getStr(ast, value3DetailArgs[0]), "Test");
    ASSERT_EQ(getDeclRef(ast, value3DetailArgs[1]), value3);
    ASSERT_EQ(getStr(ast, value3DetailArgs[2]), "OK.");
}

TEST(idlc, FieldTypeHasDeclTypeDeclaredBelow) {
    const auto [result, ast, messages] = compile("w2006.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "warning [W2006]: The field 'Api.Test.Field3' type has the declaration type enum 'Api.En' declared below "
              "at w2006:18:19");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, "Test");
    ASSERT_NE(test, HandleNone);

    ASSERT_TRUE(hasAllState(ast, test, IDL_AST_NODE_STATE_EVAULATED_BIT | IDL_AST_NODE_STATE_FORWARD_DECL_BIT));
}

TEST(idlc, ImplicitConversionFromIntToFloat) {
    const auto [result, ast, messages] = compile("w2007.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "warning [W2007]: Implicit conversion from an integer type to a floating-point type at w2007:13:30");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(test, HandleNone);

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_EQ(fields.size(), 3);

    auto fieldAttrValue0 = findChild(ast, fields[0], IDL_AST_NODE_TYPE_ATTR_VALUE);
    auto fieldAttrValue1 = findChild(ast, fields[1], IDL_AST_NODE_TYPE_ATTR_VALUE);
    auto fieldAttrValue2 = findChild(ast, fields[2], IDL_AST_NODE_TYPE_ATTR_VALUE);
    ASSERT_NE(fieldAttrValue0, HandleNone);
    ASSERT_NE(fieldAttrValue1, HandleNone);
    ASSERT_NE(fieldAttrValue2, HandleNone);

    auto value0 = findChild(ast, fieldAttrValue0, IDL_AST_NODE_TYPE_LITERAL_FLOAT);
    auto value1 = findChild(ast, fieldAttrValue1, IDL_AST_NODE_TYPE_LITERAL_INT);
    auto value2 = findChild(ast, fieldAttrValue2, IDL_AST_NODE_TYPE_LITERAL_FLOAT);
    ASSERT_NE(value0, HandleNone);
    ASSERT_NE(value1, HandleNone);
    ASSERT_NE(value2, HandleNone);

    ASSERT_DOUBLE_EQ(getFloat(ast, value0), 3.5);
    ASSERT_EQ(getInt(ast, value1), 4);
    ASSERT_DOUBLE_EQ(getFloat(ast, value2), 300.0);
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
              "tokenizer, single, version, brief, detail, author, copyright, license at e3005:6:10");
    ASSERT_EQ(messages[1],
              "error [E3005]: Invalid attribute [hex] for api 'Api' declaration, allowed attributes are cname, "
              "tokenizer, single, version, brief, detail, author, copyright, license at e3005:6:17");

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

TEST(idlc, NumberOfValuesForFieldExceedsTheArraySize) {
    const auto [result, ast, messages] = compile("e3006field");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3006]: The number of default values (4) for the field 'Api.Test.Field2' exceeds the array size "
              "(3) at e3006field:20:5");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(test, HandleNone);

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_EQ(fields.size(), 2);
    ASSERT_FALSE(hasAnyState(ast, fields[0], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_TRUE(hasAnyState(ast, fields[1], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
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
    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0], "error [E3008]: The attribute [hex] must not have arguments at e3008:10:12");
    ASSERT_EQ(messages[1], "error [E3008]: The attribute [const] must not have arguments at e3008:16:26");
    ASSERT_EQ(messages[2], "error [E3008]: The attribute [optional] must not have arguments at e3008:16:39");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto hex = findChild(ast, test, IDL_AST_NODE_TYPE_ATTR_HEX);
    ASSERT_NE(hex, HandleNone);
    ASSERT_TRUE(getChilds(ast, hex).empty());

    auto strct = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(strct, HandleNone);

    auto field = findChild(ast, strct, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_NE(field, HandleNone);

    auto cnst = findChild(ast, field, IDL_AST_NODE_TYPE_ATTR_CONST);
    ASSERT_NE(cnst, HandleNone);
    ASSERT_TRUE(getChilds(ast, cnst).empty());
    
    auto optional = findChild(ast, field, IDL_AST_NODE_TYPE_ATTR_OPTIONAL);
    ASSERT_NE(optional, HandleNone);
    ASSERT_TRUE(getChilds(ast, optional).empty());
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

TEST(idlc, AttrMustContainOneOrMoreArgs) {
    const auto [result, ast, messages] = compile("e3014.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 7);
    ASSERT_EQ(messages[0], "error [E3014]: The [brief] attribute must contain one or more arguments at e3014:1:7");
    ASSERT_EQ(messages[1], "error [E3014]: The [detail] attribute must contain one or more arguments at e3014:1:13");
    ASSERT_EQ(messages[2], "error [E3014]: The [copyright] attribute must contain one or more arguments at e3014:1:20");
    ASSERT_EQ(messages[3], "error [E3014]: The [license] attribute must contain one or more arguments at e3014:1:30");
    ASSERT_EQ(messages[4], "error [E3014]: The [author] attribute must contain one or more arguments at e3014:1:38");
    ASSERT_EQ(messages[5],
              "error [E3014]: The [tokenizer] attribute must contain one or more arguments (integers: 2, -2, 4 or "
              "string \"2-^3-4\") at e3014:1:45");
    ASSERT_EQ(messages[6], "error [E3014]: The [value] attribute must contain one or more arguments at e3014:5:18");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto attrs = getAttrs(ast, api);
    ASSERT_EQ(attrs.size(), 6);
    ASSERT_TRUE(isType(ast, attrs[0], IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF));
    ASSERT_TRUE(isType(ast, attrs[1], IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL));
    ASSERT_TRUE(isType(ast, attrs[2], IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT));
    ASSERT_TRUE(isType(ast, attrs[3], IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE));
    ASSERT_TRUE(isType(ast, attrs[4], IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR));
    ASSERT_TRUE(isType(ast, attrs[5], IDL_AST_NODE_TYPE_ATTR_TOKENIZER));
    ASSERT_TRUE(getChilds(ast, attrs[0]).empty());
    ASSERT_TRUE(getChilds(ast, attrs[1]).empty());
    ASSERT_TRUE(getChilds(ast, attrs[2]).empty());
    ASSERT_TRUE(getChilds(ast, attrs[3]).empty());
    ASSERT_TRUE(getChilds(ast, attrs[4]).empty());
    ASSERT_TRUE(getChilds(ast, attrs[5]).empty());

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto value = findChild(ast, test, IDL_AST_NODE_TYPE_CONST);
    ASSERT_NE(value, HandleNone);

    auto attrValue = findChild(ast, value, IDL_AST_NODE_TYPE_ATTR_VALUE);
    ASSERT_NE(attrValue, HandleNone);

    auto attrValueArgs = getChilds(ast, attrValue);
    ASSERT_TRUE(attrValueArgs.empty());
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
              "tokenizer, single, version, brief, detail, author, copyright, license at e3015:6:31");
    ASSERT_EQ(messages[6],
              "error [E3005]: Invalid attribute [hex] for api 'Api' declaration, allowed attributes are cname, "
              "tokenizer, single, version, brief, detail, author, copyright, license at e3015:7:39");

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

    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 6);
    ASSERT_EQ(messages[0], "error [E3014]: The [brief] attribute must contain one or more arguments at e3016:1:1");
    ASSERT_EQ(messages[1], "error [E3016]: The documentation string cannot be empty at e3016:1:1");
    ASSERT_EQ(messages[2], "error [E3014]: The [copyright] attribute must contain one or more arguments at e3016:2:5");
    ASSERT_EQ(messages[3], "error [E3016]: The documentation string cannot be empty at e3016:2:5");
    ASSERT_EQ(messages[4], "error [E3014]: The [detail] attribute must contain one or more arguments at e3016:5:9");
    ASSERT_EQ(messages[5], "error [E3016]: The documentation string cannot be empty at e3016:5:9");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto brief     = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF);
    auto copyright = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT);
    auto detail    = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL);
    ASSERT_NE(brief, HandleNone);
    ASSERT_NE(copyright, HandleNone);
    ASSERT_NE(detail, HandleNone);

    ASSERT_TRUE(getChilds(ast, brief).empty());
    ASSERT_TRUE(getChilds(ast, copyright).empty());
    ASSERT_TRUE(getChilds(ast, detail).empty());
}

TEST(idlc, MultiLineCommentMustBeSeparatedNL) {
    const auto [result, ast, messages] = compile("e3017");
    deferred(idl_compilation_result_destroy(ast));

    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3017]: The body of a multi-line comment '@ ```' must be separated by a new line at e3017:1:1");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto attrs = getAttrs(ast, api);
    ASSERT_EQ(attrs.size(), 5);
    ASSERT_TRUE(isType(ast, attrs[0], IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF));
    ASSERT_TRUE(isType(ast, attrs[1], IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL));
    ASSERT_TRUE(isType(ast, attrs[2], IDL_AST_NODE_TYPE_ATTR_DOC_COPYRIGHT));
    ASSERT_TRUE(isType(ast, attrs[3], IDL_AST_NODE_TYPE_ATTR_DOC_LICENSE));
    ASSERT_TRUE(isType(ast, attrs[4], IDL_AST_NODE_TYPE_ATTR_DOC_AUTHOR));

    auto brief = findChild(ast, api, IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF);
    ASSERT_NE(brief, HandleNone);

    ASSERT_TRUE(hasAllState(ast, brief, IDL_AST_NODE_STATE_MULTILINE_DOC_BIT));

    auto briefArgs = getChilds(ast, brief);
    ASSERT_EQ(briefArgs.size(), 3);
    ASSERT_EQ(getStr(ast, briefArgs[0]), "Brief");
    ASSERT_EQ(getStr(ast, briefArgs[1]), "\n");
    ASSERT_EQ(getStr(ast, briefArgs[2]), "Test");
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

TEST(idlc, FieldCanBeDefinedOnlyForStruct) {
    const auto [result, ast, messages] = compile("e3019.idl");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3019]: A 'field' of 'Field' can be defined only for an 'struct' at e3019:12:5");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_NE(test, HandleNone);

    auto testChilds = getChilds(ast, test);
    ASSERT_EQ(testChilds.size(), 3);
    ASSERT_TRUE(isType(ast, testChilds[0], IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF));
    ASSERT_TRUE(isType(ast, testChilds[1], IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL));
    ASSERT_TRUE(isType(ast, testChilds[2], IDL_AST_NODE_TYPE_CONST));

    auto srct = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(test, HandleNone);

    auto srctChilds = getChilds(ast, srct);
    ASSERT_EQ(srctChilds.size(), 3);
    ASSERT_TRUE(isType(ast, srctChilds[0], IDL_AST_NODE_TYPE_ATTR_DOC_BRIEF));
    ASSERT_TRUE(isType(ast, srctChilds[1], IDL_AST_NODE_TYPE_ATTR_DOC_DETAIL));
    ASSERT_TRUE(isType(ast, srctChilds[2], IDL_AST_NODE_TYPE_FIELD));
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

TEST(idlc, StructMustContainAtLeastOneField) {
    const auto [result, ast, messages] = compile("e3024");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3024]: Structure 'Api.Test' must contain at least one field at e3024:10:1");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(test, HandleNone);

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_TRUE(fields.empty());
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

TEST(idlc, FieldHasTypeOfStructDeclaredBelow) {
    const auto [result, ast, messages] = compile("e3030");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3030]: This field 'Api.Test.Field2' has the type of the structure declared below "
              "'Api.OtherStruct'; reoder declarations or use attribute [ref] at e3030:12:19");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, "Test");
    ASSERT_NE(test, HandleNone);
    ASSERT_TRUE(hasAllState(ast, test, IDL_AST_NODE_STATE_FORWARD_DECL_BIT));

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_EQ(fields.size(), 3);
    ASSERT_FALSE(hasAnyState(ast, fields[0], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_TRUE(hasAllState(ast, fields[1], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_FALSE(hasAnyState(ast, fields[2], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
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

TEST(idlc, FieldTypeCorrespondsToTypeOfStructInWhichItIsContained) {
    const auto [result, ast, messages] = compile("e3033");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3033]: The field 'Api.Test.Field2' type corresponds to the type of the structure 'Api.Test' in "
              "which it is contained; use attribute [ref] at e3033:12:19");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(test, HandleNone);
    ASSERT_FALSE(hasAnyState(ast, test, IDL_AST_NODE_STATE_FORWARD_DECL_BIT));

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_EQ(fields.size(), 3);
    ASSERT_FALSE(hasAnyState(ast, fields[0], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_TRUE(hasAllState(ast, fields[1], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_FALSE(hasAnyState(ast, fields[2], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
}

TEST(idlc, FieldCannotBeOfTypeVoid) {
    const auto [result, ast, messages] = compile("e3034field");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0], "error [E3034]: The field 'Api.Test.Field2' cannot be of type 'Void' at e3034field:12:19");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(test, HandleNone);
    ASSERT_FALSE(hasAnyState(ast, test, IDL_AST_NODE_STATE_FORWARD_DECL_BIT));

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_EQ(fields.size(), 3);
    ASSERT_FALSE(hasAnyState(ast, fields[0], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_TRUE(hasAllState(ast, fields[1], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_FALSE(hasAnyState(ast, fields[2], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
}

TEST(idlc, IsNotPossibleToAssignLiteralToType) {
    const auto [result, ast, messages] = compile("e3035");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 5);
    ASSERT_EQ(messages[0],
              "error [E3035]: It is not possible to assign string literal to type 'Api.Int32' at e3035:24:28");
    ASSERT_EQ(messages[1],
              "error [E3035]: It is not possible to assign integer literal to type 'Api.Bool' at e3035:25:27");
    ASSERT_EQ(messages[2],
              "error [E3035]: It is not possible to assign boolean literal to type 'Api.Str' at e3035:26:26");
    ASSERT_EQ(messages[3],
              "error [E3035]: It is not possible to assign float-point literal to type 'Api.Int32' at e3035:27:28");
    ASSERT_EQ(messages[4],
              "error [E3035]: It is not possible to assign enum const literal to type 'Api.Int32' at e3035:28:29");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto value2 = findChild(ast, findChild(ast, api, IDL_AST_NODE_TYPE_ENUM), "Value2");
    ASSERT_NE(value2, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(test, HandleNone);

    auto index = 0;
    auto values =
        getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD) | std::views::transform([ast, &index](const auto& field) {
        auto state = index++ < 5 ? IDL_AST_NODE_STATE_NONE_BIT : IDL_AST_NODE_STATE_BUILD_ERROR_BIT;
        EXPECT_TRUE(hasAllState(ast, field, state));
        auto attrValue = findChild(ast, field, IDL_AST_NODE_TYPE_ATTR_VALUE);
        EXPECT_NE(attrValue, HandleNone);
        auto args = getChilds(ast, attrValue);
        EXPECT_EQ(args.size(), 1);
        return getLiteral(ast, args[0]);
    });

    std::vector<Literal> expected = { "test string",          5, true, -2.54, value2,
                                      "invalid literal type", 7, true, 3.7,   value2 };

    std::vector<Literal> actual;
    actual.assign(values.begin(), values.end());

    ASSERT_EQ(actual, expected);
}

TEST(idlc, IdentifiersCaseSensitive) {
    const auto [result, ast, messages] = compile("e3036");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0],
              "error [E3036]: Identifiers are case sensitive, error in 'ApI', but expected 'Api' at e3036:1:9");
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
    ASSERT_EQ(messages[0], "error [E3037]: Symbol definition 'Test' not found at e3037:1:9");

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

TEST(idlc, NameOrTypeMustStartWithCapitalLetter) {
    const auto [result, ast, messages] = compile("e3047");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 3);
    ASSERT_EQ(messages[0], "error [E3047]: The name or type must start with a capital letter 'tEst' at e3047:9:6");
    ASSERT_EQ(messages[1], "error [E3047]: The name or type must start with a capital letter 'fail' at e3047:14:11");
    ASSERT_EQ(messages[2], "error [E3047]: The name or type must start with a capital letter '0Other' at e3047:17:6");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto enums = getChilds(ast, api, IDL_AST_NODE_TYPE_ENUM);
    ASSERT_EQ(enums.size(), 3);
    ASSERT_EQ(getStr(ast, enums[0]), "tEst");
    ASSERT_EQ(getStr(ast, findChild(ast, enums[1], IDL_AST_NODE_TYPE_CONST)), "fail");
    ASSERT_EQ(getStr(ast, enums[2]), "0Other");
}

TEST(idlc, OnlyLiteralsAndCompileTimeExprEnumConstCanBeUsedAsDefaultValues) {
    const auto [result, ast, messages] = compile("e3048");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3048]: Only literals and compile-time expressions (enumeration constants) can be used as "
              "default values at e3048:18:25");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(test, HandleNone);

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_EQ(fields.size(), 2);
    ASSERT_FALSE(hasAnyState(ast, fields[0], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_TRUE(hasAnyState(ast, fields[1], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));

    auto attrValue0 = findChild(ast, fields[0], IDL_AST_NODE_TYPE_ATTR_VALUE);
    auto attrValue1 = findChild(ast, fields[1], IDL_AST_NODE_TYPE_ATTR_VALUE);
    ASSERT_NE(attrValue0, HandleNone);
    ASSERT_NE(attrValue1, HandleNone);

    auto args0 = getChilds(ast, attrValue0);
    auto args1 = getChilds(ast, attrValue1);
    ASSERT_EQ(args0.size(), 1);
    ASSERT_EQ(args1.size(), 1);

    auto actual0 = getLiteral(ast, args0.front());
    auto actual1 = getLiteral(ast, args1.front());

    Literal expected0 = 7;
    Literal expected1 = findChild(ast, api, IDL_AST_NODE_TYPE_ENUM);

    ASSERT_EQ(actual0, expected0);
    ASSERT_EQ(actual1, expected1);
}

TEST(idlc, CannotAssignConstOfTypeToDeclWithDifferentEnumType) {
    const auto [result, ast, messages] = compile("e3049");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3049]: Cannot assign a constant 'Value2' of 'Api.En2' type to field with a different "
              "enumeration type 'Api.En1' at e3049:24:26");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(test, HandleNone);

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_EQ(fields.size(), 2);
    ASSERT_FALSE(hasAnyState(ast, fields[0], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_TRUE(hasAnyState(ast, fields[1], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));

    auto attrValue0 = findChild(ast, fields[0], IDL_AST_NODE_TYPE_ATTR_VALUE);
    auto attrValue1 = findChild(ast, fields[1], IDL_AST_NODE_TYPE_ATTR_VALUE);
    ASSERT_NE(attrValue0, HandleNone);
    ASSERT_NE(attrValue1, HandleNone);

    auto args0 = getChilds(ast, attrValue0);
    auto args1 = getChilds(ast, attrValue1);
    ASSERT_EQ(args0.size(), 1);
    ASSERT_EQ(args1.size(), 1);

    auto actual0 = getLiteral(ast, args0.front());
    auto actual1 = getLiteral(ast, args1.front());

    Literal expected0 = 4;
    Literal expected1 = findChild(ast, findChild(ast, api, "En2"), "Value2");

    ASSERT_EQ(actual0, expected0);
    ASSERT_EQ(actual1, expected1);
}

TEST(idlc, AttributeMustContainOneArgument) {
    const auto [result, ast, messages] = compile("e3050");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0],
              "error [E3050]: The [array] attribute must contain one argument (fixed-size integer or reference to an "
              "integer field/arg specifying the size) at e3050:12:27");
    ASSERT_EQ(messages[1],
              "error [E3050]: The [array] attribute must contain one argument (fixed-size integer or reference to an "
              "integer field/arg specifying the size) at e3050:13:27");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(test, HandleNone);

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_EQ(fields.size(), 3);
    ASSERT_EQ(findChild(ast, fields[0], IDL_AST_NODE_TYPE_ATTR_ARRAY), HandleNone);
    ASSERT_NE(findChild(ast, fields[1], IDL_AST_NODE_TYPE_ATTR_ARRAY), HandleNone);
    ASSERT_NE(findChild(ast, fields[2], IDL_AST_NODE_TYPE_ATTR_ARRAY), HandleNone);
}

TEST(idlc, ArraySizeArgumentMustBePositive) {
    const auto [result, ast, messages] = compile("e3051");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0], "error [E3051]: The [array] size argument must be positive at e3051:11:27");
    ASSERT_EQ(messages[1], "error [E3051]: The [array] size argument must be positive at e3051:13:27");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(test, HandleNone);

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_EQ(fields.size(), 3);

    auto arr0 = findChild(ast, fields[0], IDL_AST_NODE_TYPE_ATTR_ARRAY);
    auto arr1 = findChild(ast, fields[1], IDL_AST_NODE_TYPE_ATTR_ARRAY);
    auto arr2 = findChild(ast, fields[2], IDL_AST_NODE_TYPE_ATTR_ARRAY);
    ASSERT_NE(arr0, HandleNone);
    ASSERT_NE(arr1, HandleNone);
    ASSERT_NE(arr2, HandleNone);

    auto arg1 = getLiteral(ast, findChild(ast, arr1, IDL_AST_NODE_TYPE_LITERAL));
    ASSERT_TRUE(getChilds(ast, arr0).empty());
    ASSERT_TRUE(getChilds(ast, arr2).empty());
    ASSERT_EQ(arg1, Literal(3));
}

TEST(idlc, ArrayAttrMustReferToIntField) {
    const auto [result, ast, messages] = compile("e3052field");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 2);
    ASSERT_EQ(messages[0],
              "error [E3052]: The [array] attribute must refer to an integer field, but the reference to the field "
              "'Api.Test.Count' has the type 'Api.Float32' at e3052field:11:5");
    ASSERT_EQ(messages[1],
              "error [E3052]: The [array] attribute must refer to an integer field, but the reference to the field "
              "'Api.Test.Count2' has the type array of 'Api.Int16' at e3052field:14:5");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(test, HandleNone);

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_EQ(fields.size(), 4);

    ASSERT_TRUE(hasAnyState(ast, fields[0], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_FALSE(hasAnyState(ast, fields[1], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_TRUE(hasAnyState(ast, fields[2], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_FALSE(hasAnyState(ast, fields[3], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
}

TEST(idlc, RefToTheFieldArraySizeMustBeInTheSameScope) {
    const auto [result, ast, messages] = compile("e3053field");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3053]: The reference to the array size must be in the same scope as field 'Api.Test.Arr' at "
              "e3053field:16:5");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, "Test");
    ASSERT_NE(test, HandleNone);

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_EQ(fields.size(), 1);
    ASSERT_TRUE(hasAnyState(ast, fields[0], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
}

TEST(idlc, ArraySizeRefCannotReferToItsOwnDeclaration) {
    const auto [result, ast, messages] = compile("e3054");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3054]: An array size reference cannot refer to its own field 'Api.Test.Arr' at e3054:11:5");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(test, HandleNone);

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_EQ(fields.size(), 1);
    ASSERT_TRUE(hasAnyState(ast, fields[0], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
}

TEST(idlc, MultipleDefaultValuesCanBeAssignedOnlyToArray) {
    const auto [result, ast, messages] = compile("e3055field");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3055]: Multiple default values can be assigned to field 'Api.Test.Field2' only to an array at "
              "e3055field:12:5");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, IDL_AST_NODE_TYPE_STRUCT);
    ASSERT_NE(test, HandleNone);

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_EQ(fields.size(), 2);
    ASSERT_FALSE(hasAnyState(ast, fields[0], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_TRUE(hasAnyState(ast, fields[1], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
}

TEST(idlc, MultipleEnumConstsCanBeAssignedOnlyToFlagEnum) {
    const auto [result, ast, messages] = compile("e3056field");
    deferred(idl_compilation_result_destroy(ast));
    ASSERT_EQ(result, IDL_RESULT_SUCCESS);
    ASSERT_EQ(messages.size(), 1);
    ASSERT_EQ(messages[0],
              "error [E3056]: Multiple enumeration constants can be assigned to field 'Api.Test.Field4' only to a flag "
              "enumeration at e3056field:26:5");

    auto api = idl_compilation_result_get_api(ast);
    ASSERT_NE(api, HandleNone);

    auto test = findChild(ast, api, "Test");
    ASSERT_NE(test, HandleNone);

    auto fields = getChilds(ast, test, IDL_AST_NODE_TYPE_FIELD);
    ASSERT_EQ(fields.size(), 4);
    ASSERT_FALSE(hasAnyState(ast, fields[0], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_FALSE(hasAnyState(ast, fields[1], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_FALSE(hasAnyState(ast, fields[2], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
    ASSERT_TRUE(hasAnyState(ast, fields[3], IDL_AST_NODE_STATE_BUILD_ERROR_BIT));
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
