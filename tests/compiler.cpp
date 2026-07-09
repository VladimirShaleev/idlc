#include "compiler.hpp"
#include "finally.hpp"

#include <cmrc/cmrc.hpp>
CMRC_DECLARE(idlc::cases);

using namespace std::string_literals;

static idl_source_t* import(idl_utf8_t name, idl_uint32_t depth, idl_data_t data) {
    auto fs = cmrc::idlc::cases::get_filesystem();
    try {
        auto filename = std::string(name);
        if (filename.length() > 0) {
            filename[0] = std::toupper(filename[0]);
        }
        auto file = fs.open("cases/"s + filename + ".idl"s);
        return new idl_source_t{ name, (const char*) file.begin(), idl_uint32_t(file.size()) };
    } catch (const std::system_error& exc) {
        if (exc.code() == make_error_code(std::errc::no_such_file_or_directory)) {
            return nullptr;
        }
        throw;
    }
}

static void releaseImport(idl_source_t* source, idl_data_t data) {
    delete source;
}

std::pair<idl_result_t, std::vector<std::string>> compile(std::string_view testCase, bool warnAsErrors) {
    idl_options_t options{};
    auto code = idl_options_create(&options);
    // ASSERT_EQ(code, IDL_RESULT_SUCCESS);
    deferred(idl_options_destroy(options));
    idl_options_set_debug_mode(options, 0);
    idl_options_set_warnings_as_errors(options, warnAsErrors);
    idl_options_set_importer(options, import, nullptr);
    idl_options_set_release_import(options, releaseImport, nullptr);

    idl_compiler_t compiler{};
    idl_compiler_create(&compiler);
    // ASSERT_EQ(code, IDL_RESULT_SUCCESS);
    deferred(idl_compiler_destroy(compiler));

    idl_compilation_result_t result{};
    code = idl_compiler_compile(compiler, IDL_GENERATOR_C, testCase.data(), 0, nullptr, options, &result);

    idl_uint32_t count{};
    idl_compilation_result_get_messages(result, &count, nullptr);
    std::vector<idl_message_t> messages;

    messages.resize(count);
    idl_compilation_result_get_messages(result, &count, messages.data());
    deferred(idl_compilation_result_destroy(result));

    std::vector<std::string> results;
    results.reserve(count);

    for (const auto& message : messages) {
        std::ostringstream ss;
        std::string status;
        char prefixStatus;
        if (message.status >= IDL_STATUS_E3001) {
            status       = "error";
            prefixStatus = 'E';
        } else if (message.status >= IDL_STATUS_W2001) {
            status       = "warning";
            prefixStatus = 'W';
        } else {
            status       = "note";
            prefixStatus = 'N';
        }
        ss << status << " [" << prefixStatus << (int) message.status << "]: " << message.message;
        if (message.line > 0) {
            ss << " at " << message.filename << ':' << message.line << ':' << message.column;
        }
        results.push_back(ss.str());
    }

    return { code, results };
}
