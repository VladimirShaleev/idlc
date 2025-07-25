#include "compilation_result.hpp"
#include "options.hpp"
#include "parser.hpp"
#include "scanner.hpp"

void generateC(idl::Context& ctx,
               const std::filesystem::path& out,
               idl_write_callback_t writer,
               idl_data_t writerData,
               std::span<idl_utf8_t> additions);

void generateJs(idl::Context& ctx,
                const std::filesystem::path& out,
                idl_write_callback_t writer,
                idl_data_t writerData);

struct _idl_compiler : public idl::Object {};

namespace idl {

class Compiler final : public _idl_compiler {
public:
    idl_result_t compile(idl_generator_t generator,
                         idl_utf8_t file,
                         std::span<const idl_source_t> sources,
                         Options* options,
                         CompilationResult* result) noexcept {
        try {
            Context context{ options, result };
            Scanner scanner{ context, options, sources, file ? file : "" };
            Parser parser{ scanner };
#if YYDEBUG
            parser.set_debug_level(options && options->getDebugMode() ? 1 : 0);
#endif
            auto code = parser.parse();

            if (code != 0) {
                if (result) {
                    Exception exc(IDL_STATUS_E2011, "<input>", 0, 0, "unknown error");
                    result->addMessage(exc);
                    return IDL_RESULT_SUCCESS;
                } else {
                    return IDL_RESULT_ERROR_COMPILATION;
                }
            }

            context.prepareEnumConsts();
            context.prepareStructs();
            context.prepareCallbacks();
            context.prepareFunctions();
            context.prepareMethods();
            context.prepareProperties();
            context.prepareEvents();
            context.prepareInterfaces();
            context.prepareHandles();
            context.prepareDocumentation();

            auto output = std::filesystem::current_path();
            idl_write_callback_t writer{};
            idl_data_t writerData{};
            std::vector<idl_utf8_t> additions{};
            if (options) {
                output = options->getOutputDir();
                writer = options->getWriter(&writerData);
                idl_uint32_t num{};
                options->getAdditions(num, nullptr);
                additions.resize(num);
                options->getAdditions(num, additions.data());
                if (auto version = options->getVersion()) {
                    context.apiVersion(*version);
                }
            }

            switch (generator) {
                case IDL_GENERATOR_C:
                    generateC(context, output, writer, writerData, std::span{ additions.data(), additions.size() });
                    break;
                case IDL_GENERATOR_JAVA_SCRIPT:
#ifdef IDLC_SUPPORTED_JS
                    generateJs(context, output, writer, writerData);
                    break;
#else
                    return IDL_RESULT_ERROR_NOT_SUPPORTED;
#endif
                default:
                    assert(!"unreachable code");
                    break;
            }
        } catch (const Exception& exc) {
            if (result) {
                result->addMessage(exc);
            } else {
                return exc.status() == IDL_STATUS_E2067 ? IDL_RESULT_ERROR_FILE_CREATE : IDL_RESULT_ERROR_COMPILATION;
            }
        } catch (const std::bad_alloc&) {
            if (result) {
                Exception exc(IDL_STATUS_E2045, "<input>", 0, 0, "out of memory");
                result->addMessage(exc);
            } else {
                return IDL_RESULT_ERROR_OUT_OF_MEMORY;
            }
        } catch (...) {
            if (result) {
                Exception exc(IDL_STATUS_E2011, "<input>", 0, 0, "unknown error");
                result->addMessage(exc);
            } else {
                return IDL_RESULT_ERROR_UNKNOWN;
            }
        }
        return IDL_RESULT_SUCCESS;
    }
};

}; // namespace idl

idl_uint32_t idl_version(void) {
    return IDL_VERSION;
}

idl_utf8_t idl_version_string(void) {
    return IDL_VERSION_STRING;
}

idl_utf8_t idl_result_to_string(idl_result_t result) {
    switch (result) {
        case IDL_RESULT_SUCCESS:
            return "no error has occurred";
        case IDL_RESULT_ERROR_UNKNOWN:
            return "unknown error";
        case IDL_RESULT_ERROR_OUT_OF_MEMORY:
            return "out of memory";
        case IDL_RESULT_ERROR_INVALID_ARG:
            return "invalid argument";
        case IDL_RESULT_ERROR_FILE_CREATE:
            return "failed to create file";
        case IDL_RESULT_ERROR_COMPILATION:
            return "compilation failed";
        case IDL_RESULT_ERROR_NOT_SUPPORTED:
            return "not supported";
        default:
            return "<unknown result>";
    }
}

idl_result_t idl_options_create(idl_options_t* options) {
    assert(options);
    return idl::Object::create<idl::Options>(*options);
}

idl_options_t idl_options_reference(idl_options_t options) {
    assert(options);
    options->reference();
    return options;
}

void idl_options_destroy(idl_options_t options) {
    if (options) {
        options->destroy();
    }
}

idl_bool_t idl_options_get_debug_mode(idl_options_t options) {
    assert(options);
    return options->as<idl::Options>()->getDebugMode() ? 1 : 0;
}

void idl_options_set_debug_mode(idl_options_t options, idl_bool_t enable) {
    assert(options);
    options->as<idl::Options>()->setDebugMode(enable);
}

idl_bool_t idl_options_get_warnings_as_errors(idl_options_t options) {
    assert(options);
    return options->as<idl::Options>()->getWarningsAsErrors() ? 1 : 0;
}

void idl_options_set_warnings_as_errors(idl_options_t options, idl_bool_t enable) {
    assert(options);
    options->as<idl::Options>()->setWarningsAsErrors(enable);
}

idl_utf8_t idl_options_get_output_dir(idl_options_t options) {
    assert(options);
    return options->as<idl::Options>()->getOutputDir();
}

void idl_options_set_output_dir(idl_options_t options, idl_utf8_t dir) {
    assert(options);
    options->as<idl::Options>()->setOutputDir(dir);
}

void idl_options_get_import_dirs(idl_options_t options, idl_uint32_t* dir_count, idl_utf8_t* dirs) {
    assert(options);
    assert(dir_count);
    return options->as<idl::Options>()->getImportDirs(*dir_count, dirs);
}

void idl_options_set_import_dirs(idl_options_t options, idl_uint32_t dir_count, const idl_utf8_t* dirs) {
    assert(options);
    options->as<idl::Options>()->setImportDirs(std::span{ dirs, dir_count });
}

idl_import_callback_t idl_options_get_importer(idl_options_t options, idl_data_t* data) {
    assert(options);
    return options->as<idl::Options>()->getImporter(data);
}

void idl_options_set_importer(idl_options_t options, idl_import_callback_t callback, idl_data_t data) {
    assert(options);
    options->as<idl::Options>()->setImporter(callback, data);
}

idl_release_import_callback_t idl_options_get_release_import(idl_options_t options, idl_data_t* data) {
    assert(options);
    return options->as<idl::Options>()->getReleaseImport(data);
}

void idl_options_set_release_import(idl_options_t options, idl_release_import_callback_t callback, idl_data_t data) {
    assert(options);
    return options->as<idl::Options>()->setReleaseImport(callback, data);
}

idl_write_callback_t idl_options_get_writer(idl_options_t options, idl_data_t* data) {
    assert(options);
    return options->as<idl::Options>()->getWriter(data);
}

void idl_options_set_writer(idl_options_t options, idl_write_callback_t callback, idl_data_t data) {
    assert(options);
    return options->as<idl::Options>()->setWriter(callback, data);
}

void idl_options_get_additions(idl_options_t options, idl_uint32_t* addition_count, idl_utf8_t* additions) {
    assert(options);
    assert(addition_count);
    return options->as<idl::Options>()->getAdditions(*addition_count, additions);
}

void idl_options_set_additions(idl_options_t options, idl_uint32_t addition_count, const idl_utf8_t* additions) {
    assert(options);
    options->as<idl::Options>()->setAdditions(std::span{ additions, addition_count });
}

const idl_api_version_t* idl_options_get_version(idl_options_t options) {
    assert(options);
    return options->as<idl::Options>()->getVersion();
}

void idl_options_set_version(idl_options_t options, const idl_api_version_t* version) {
    assert(options);
    options->as<idl::Options>()->setVersion(version);
}

idl_result_t idl_compiler_create(idl_compiler_t* compiler) {
    assert(compiler);
    return idl::Object::create<idl::Compiler>(*compiler);
}

idl_compiler_t idl_compiler_reference(idl_compiler_t compiler) {
    assert(compiler);
    compiler->reference();
    return compiler;
}

void idl_compiler_destroy(idl_compiler_t compiler) {
    if (compiler) {
        compiler->destroy();
    }
}

idl_result_t idl_compiler_compile(idl_compiler_t compiler,
                                  idl_generator_t generator,
                                  idl_utf8_t file,
                                  idl_uint32_t source_count,
                                  const idl_source_t* sources,
                                  idl_options_t options,
                                  idl_compilation_result_t* result) {
    if (result) {
        const auto resultCode = idl::Object::create<idl::CompilationResult>(*result);
        if (resultCode != IDL_RESULT_SUCCESS) {
            return resultCode;
        }
    }
    return compiler->as<idl::Compiler>()->compile(generator,
                                                  file,
                                                  std::span{ sources, source_count },
                                                  options ? options->as<idl::Options>() : nullptr,
                                                  result ? (*result)->as<idl::CompilationResult>() : nullptr);
}

idl_compilation_result_t idl_compilation_result_reference(idl_compilation_result_t compilation_result) {
    assert(compilation_result);
    compilation_result->reference();
    return compilation_result;
}

void idl_compilation_result_destroy(idl_compilation_result_t compilation_result) {
    if (compilation_result) {
        compilation_result->destroy();
    }
}

idl_bool_t idl_compilation_result_has_warnings(idl_compilation_result_t compilation_result) {
    assert(compilation_result);
    return compilation_result->as<idl::CompilationResult>()->hasWarnings();
}

idl_bool_t idl_compilation_result_has_errors(idl_compilation_result_t compilation_result) {
    assert(compilation_result);
    return compilation_result->as<idl::CompilationResult>()->hasErrors();
}

void idl_compilation_result_get_messages(idl_compilation_result_t compilation_result,
                                         idl_uint32_t* message_count,
                                         idl_message_t* messages) {
    assert(compilation_result);
    assert(message_count);
    return compilation_result->as<idl::CompilationResult>()->getMessages(*message_count, messages);
}
