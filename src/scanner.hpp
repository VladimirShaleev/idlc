#ifndef SCANNER_HPP
#define SCANNER_HPP

#include "context.hpp"
#include "options.hpp"
#include "parser.hpp"

#include <fstream>

#if !defined(yyFlexLexerOnce)
# include <FlexLexer.h>
#endif

namespace idl {

class Scanner : public yyFlexLexer {
public:
    Scanner(Context& ctx,
            const Options* options,
            std::span<const idl_source_t> sources,
            const std::filesystem::path& file) :
        yyFlexLexer(),
        _ctx(ctx),
        _options(options),
        _sources(sources) {
        const std::string str = "<input>";
        const auto loc        = idl::location(idl::position(&str, 1, 1));

        if (_basePath.empty()) {
            _basePath = std::filesystem::current_path();
        }

        std::filesystem::path path{};
        if (!file.empty()) {
            path      = file.is_relative() ? std::filesystem::current_path() / file : file;
            _basePath = path.parent_path();
        }

        import(loc, path, false);
    }

    int yylex(Parser::semantic_type* yylval, Parser::location_type* yylloc);

    Context& context() noexcept {
        return _ctx;
    }

    const std::string* filename() const noexcept {
        return _imports.back()->filename;
    }

    void import(const idl::location& loc, const std::filesystem::path& file, bool isRelative = true) {
        if (isRelative && file.is_absolute()) {
            err<IDL_RESULT_E2041>(loc, file.string());
        }
        const auto [path, source, needRelease] = findFile(loc, file);
        const auto filename = path.is_absolute() ? std::filesystem::relative(path, _basePath).string() : path.string();

        if (_allImports.contains(filename)) {
            return;
        }
        _allImports[filename] = std::make_unique<std::string>(filename);
        auto filenamePtr      = _allImports[filename].get();

        if (!_imports.empty()) {
            _imports.back()->location = loc;
            _imports.back()->line     = yylineno;
        }
        auto initLineNum = _imports.empty() ? 1 : std::numeric_limits<position::counter_type>::max() - 1;
        _imports.emplace_back(std::make_unique<Import>(this,
                                                       source,
                                                       needRelease,
                                                       path,
                                                       filenamePtr,
                                                       idl::location(idl::position(filenamePtr, initLineNum, 1)),
                                                       1,
                                                       nullptr,
                                                       nullptr));
        auto& import = *_imports.back();
        if (import.source) {
            struct MemoryBuffer : public std::streambuf {
            public:
                MemoryBuffer(const char* data, size_t size) {
                    setg(const_cast<char*>(data), const_cast<char*>(data), const_cast<char*>(data) + size);
                }
            };

            import.stream = new std::istream(new MemoryBuffer(import.source->data, (size_t) import.source->size));
        } else {
            import.stream = new std::ifstream(path);
            if (import.stream->fail()) {
                delete import.stream;
                err<IDL_RESULT_E2042>(loc, path.string());
                return;
            }
        }
        import.buffer = yy_create_buffer(import.stream, 16384);
        yy_switch_to_buffer(import.buffer);

        yylineno = 1;

        _needUpdateLoc = true;
    }

    bool popImport() {
        if (_imports.size() > 1) {
            auto& import = *(_imports.rbegin() + 1);
            yy_switch_to_buffer(import->buffer);
            yylineno = import->line;
        }
        assert(!_imports.empty());
        if (_imports.back()->releaseSource && _options) {
            idl_data_t data{};
            if (auto callback = _options->getReleaseImport(&data)) {
                callback(const_cast<idl_source_t*>(_imports.back()->source), data);
            }
        }
        _imports.pop_back();
        _needUpdateLoc = true;
        if (_imports.size() > 0) {
            _ctx.popFile();
        }
        return !_imports.empty();
    }

    void action(idl::location& loc) {
        if (!_needUpdateLoc) {
            loc.step();
            loc.columns(yyleng);
        } else {
            loc            = _imports.back()->location;
            _needUpdateLoc = false;
        }
    }

    int lineIndent = -1;

private:
    struct Import {
        ~Import() {
            scanner->yy_delete_buffer(buffer);
            buffer = nullptr;
            if (stream) {
                std::streambuf* buf{};
                if (source) {
                    buf = stream->rdbuf();
                }
                delete stream;
                stream = nullptr;
                if (buf) {
                    delete buf;
                    buf = nullptr;
                }
            }
        }

        Scanner* scanner{};
        const idl_source_t* source{};
        bool releaseSource{};
        std::filesystem::path file;
        std::string* filename;
        idl::location location;
        int line{};
        yy_buffer_state* buffer{};
        std::istream* stream{};
    };

    std::tuple<std::filesystem::path, const idl_source_t*, bool> findFile(const idl::location& loc,
                                                                          const std::filesystem::path& file) const {
        if (file.empty() && !_sources.empty()) {
            auto& source = _sources.front();
            return { source.name, &source, false };
        }
        auto name = normalize(file);

        if (_options) {
            idl_data_t data{};
            if (auto importer = _options->getImporter(&data)) {
                if (auto source = importer(name.c_str(), (idl_uint32_t) _imports.size(), data)) {
                    return { name, source, true };
                }
            }
        }

        for (const auto& source : _sources) {
            auto sourceName = normalize(source.name);
            if (name == sourceName) {
                return { name, &source, false };
            }
        }

        if (file.is_absolute()) {
            if (!std::filesystem::exists(file) || !std::filesystem::is_regular_file(file)) {
                err<IDL_RESULT_E2041>(loc, file.string());
            }
            return { file, nullptr, false };
        }

        std::vector<std::filesystem::path> importDirs{};
        if (_options) {
            idl_uint32_t dirCount{};
            _options->getImportDirs(dirCount, nullptr);
            std::vector<idl_utf8_t> dirs;
            dirs.resize(dirCount);
            _options->getImportDirs(dirCount, dirs.data());
            importDirs.reserve(dirs.size());
            for (const auto dir : dirs) {
                importDirs.push_back(dir);
            }
        }
        importDirs.push_back(_basePath);

        for (const auto basePath : importDirs) {
            auto fullpath = basePath / file;
            auto filename = file.string();
            while (true) {
                if (!fullpath.has_extension()) {
                    fullpath.replace_extension(".idl");
                } else if (lowercase(fullpath.extension()) != ".idl") {
                    fullpath += ".idl";
                }
                if (std::filesystem::exists(fullpath) && std::filesystem::is_regular_file(fullpath)) {
                    return { fullpath, nullptr, false };
                }
                for (const auto& entry : std::filesystem::directory_iterator(fullpath.parent_path())) {
                    const auto pathActual   = lowercase(entry.path());
                    const auto pathExpected = lowercase(fullpath);
                    if (pathActual == pathExpected) {
                        return { entry.path(), nullptr, false };
                    }
                }
                const auto offset = filename.find('.');
                if (offset == std::string::npos) {
                    break;
                }
                filename.replace(offset, 1, 1, '/');
                fullpath = basePath / std::filesystem::path(filename).make_preferred();
            }
        }
        err<IDL_RESULT_E2041>(loc, file.string());
    }

    std::string normalize(const std::filesystem::path& path) const {
        auto filename = path.is_absolute() ? std::filesystem::relative(path, _basePath).string() : path.string();
        auto offset   = filename.find('\\');
        while (offset != std::string::npos) {
            filename.replace(offset, 1, 1, '/');
            offset = filename.find('\\', offset + 1);
        }
        lower(filename);
        if (offset = filename.rfind(".idl"); offset != std::string::npos) {
            filename = filename.substr(0, offset);
        }
        return filename;
    }

    static std::string lowercase(const std::filesystem::path& path) {
        auto str = std::filesystem::path{ path }.make_preferred().string();
        std::transform(str.begin(), str.end(), str.begin(), [](auto c) {
            return std::tolower(c);
        });
        return str;
    }

    Context& _ctx;
    const Options* _options;
    std::span<const idl_source_t> _sources;
    std::filesystem::path _basePath{};
    std::vector<std::unique_ptr<Import>> _imports{};
    std::map<std::string, std::unique_ptr<std::string>> _allImports{};
    bool _needUpdateLoc{};
};

} // namespace idl

#endif
