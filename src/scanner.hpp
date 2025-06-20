#ifndef SCANNER_HPP
#define SCANNER_HPP

#include "ast.hpp"
#include "context.hpp"
#include "parser.hpp"

#include <fstream>

#if !defined(yyFlexLexerOnce)
# include <FlexLexer.h>
#endif

namespace idl {

class Scanner : public yyFlexLexer {
public:
    Scanner(Context& ctx, const std::filesystem::path& file) : yyFlexLexer(), _ctx(ctx) {
        const std::string str = "<input>";
        const auto loc        = idl::location(idl::position(&str, 1, 1));

        const auto path = file.is_relative() ? std::filesystem::current_path() / file : file;
        _basePath       = path.parent_path();

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
            err<E2041>(loc, file.string());
        }
        const auto path     = findFile(loc, file);
        const auto filename = std::filesystem::relative(path, _basePath).string();

        if (_allImports.contains(filename)) {
            return;
        }
        _allImports[filename] = std::make_unique<std::string>(filename);
        auto filenamePtr      = _allImports[filename].get();

        if (!_imports.empty()) {
            _imports.back()->location = loc;
            _imports.back()->line     = yylineno;
        }
        _imports.emplace_back(std::make_unique<Import>(
            this, path, filenamePtr, idl::location(idl::position(filenamePtr, 1, 1)), 1, nullptr, nullptr));
        auto& import  = *_imports.back();
        import.stream = new std::ifstream(path);
        if (import.stream->fail()) {
            delete import.stream;
            err<E2042>(loc, path.string());
            return;
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
        _imports.pop_back();
        _needUpdateLoc = true;
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
                stream->close();
                delete stream;
                stream = nullptr;
            }
        }

        Scanner* scanner{};
        std::filesystem::path file;
        std::string* filename;
        idl::location location;
        int line{};
        yy_buffer_state* buffer{};
        std::ifstream* stream{};
    };

    std::filesystem::path findFile(const idl::location& loc, const std::filesystem::path& file) const {
        if (file.is_absolute()) {
            if (!std::filesystem::exists(file) || !std::filesystem::is_regular_file(file)) {
                err<E2041>(loc, file.string());
            }
            return file;
        }
        auto fullpath = _basePath / file;
        auto filename = file.string();
        while (true) {
            if (!fullpath.has_extension()) {
                fullpath.replace_extension(".idl");
            } else if (lowercase(fullpath.extension()) != ".idl") {
                fullpath += ".idl";
            }
            if (std::filesystem::exists(fullpath) && std::filesystem::is_regular_file(fullpath)) {
                return fullpath;
            }
            for (const auto& entry : std::filesystem::directory_iterator(fullpath.parent_path())) {
                const auto pathActual   = lowercase(entry.path());
                const auto pathExpected = lowercase(fullpath);
                if (pathActual == pathExpected) {
                    return entry.path();
                }
            }
            const auto offset = filename.find('.');
            if (offset == std::string::npos) {
                break;
            }
            filename.replace(offset, 1, 1, '/');
            fullpath = _basePath / std::filesystem::path(filename).make_preferred();
        }
        err<E2041>(loc, file.string());
    }

    static std::string lowercase(const std::filesystem::path& path) {
        auto str = std::filesystem::path{ path }.make_preferred().string();
        std::transform(str.begin(), str.end(), str.begin(), [](auto c) {
            return std::tolower(c);
        });
        return str;
    }

    Context& _ctx;
    std::filesystem::path _basePath{};
    std::vector<std::unique_ptr<Import>> _imports{};
    std::map<std::string, std::unique_ptr<std::string>> _allImports{};
    bool _needUpdateLoc{};
};

} // namespace idl

#endif
