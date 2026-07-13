#ifndef IDL_WRITER_HPP
#define IDL_WRITER_HPP

#include "ast_ref.hpp"
#include "options.hpp"

namespace idl {

class Writer;

class Output {
public:
    explicit operator bool() const noexcept {
        return _out->stream != nullptr;
    }

    [[nodiscard]] std::ostream& stream() noexcept {
        return *_out->stream;
    }

    [[nodiscard]] ASTNodeRef api() const noexcept {
        return _out->api;
    }

    [[nodiscard]] CompilationResultBase* result() noexcept {
        return api().result();
    }

    [[nodiscard]] Options* options() noexcept {
        return _out->options;
    }

    [[nodiscard]] const std::filesystem::path& dir() const noexcept {
        return _out->dir;
    }

    [[nodiscard]] const std::string& filename() const noexcept {
        return _out->filename;
    }

private:
    friend Writer;

    struct OutputImpl {
        OutputImpl(Options* options,
                   ASTNodeRef api,
                   const std::filesystem::path& dir,
                   const std::string& filename,
                   idl_write_callback_t writer,
                   idl_data_t writerData) noexcept :
            options(options),
            api(api),
            dir(dir),
            filename(filename),
            writer(writer),
            writerData(writerData) {
            if (writer) {
                sstream = std::make_unique<std::ostringstream>();
                stream  = sstream.get();
            } else {
                std::filesystem::create_directories(dir);
                fstream = std::make_unique<std::ofstream>(dir / filename);
                if (fstream->fail()) {
                    // result->addMessage();
                    return;
                }
                stream = fstream.get();
            }
        }

        ~OutputImpl() {
            if (stream && writer) {
                try {
                    const auto data = sstream->str();
                    idl_source_t source{ filename.c_str(), data.c_str(), (idl_uint32_t) data.length() };
                    writer(&source, writerData);
                } catch (const std::bad_alloc) {
                    // result->addMessage();
                } catch (...) {
                    // result->addMessage();
                }
            }
        }

        Options* options{};
        ASTNodeRef api{};
        std::filesystem::path dir{};
        std::string filename{};
        idl_write_callback_t writer{};
        idl_data_t writerData{};
        std::unique_ptr<std::ofstream> fstream{};
        std::unique_ptr<std::ostringstream> sstream{};
        std::ostream* stream{};
    };

    Output(Options* options,
           ASTNodeRef api,
           const std::filesystem::path& dir,
           const std::string& filename,
           idl_write_callback_t writer,
           idl_data_t writerData) noexcept :
        _out(std::make_shared<OutputImpl>(options, api, dir, filename, writer, writerData)) {
    }

    std::shared_ptr<OutputImpl> _out{};
};

class Writer {
public:
    Writer(Options* options, ASTNodeRef api) : _options(options), _api(api) {
        _out = std::filesystem::current_path();
        if (_options) {
            _out    = options->getOutputDir();
            _writer = options->getWriter(&_writerData);
        }
    }

    [[nodiscard]] Output createOutput(const std::string& filename) noexcept {
        return Output(_options, _api, _out, filename, _writer, _writerData);
    }

    [[nodiscard]] CompilationResultBase* result() noexcept {
        return _api.result();
    }

    [[nodiscard]] Options* options() noexcept {
        return _options;
    }

    [[nodiscard]] const std::filesystem::path& dir() const noexcept {
        return _out;
    }

    Writer(const Writer&) = delete;
    Writer(Writer&&)      = delete;

    Writer& operator=(const Writer&) = delete;
    Writer& operator=(Writer&&)      = delete;

private:
    Options* _options{};
    ASTNodeRef _api{};
    std::filesystem::path _out{};
    idl_write_callback_t _writer{};
    idl_data_t _writerData{};
};

} // namespace idl

#endif
