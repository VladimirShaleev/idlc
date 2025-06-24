#ifndef IDL_OPTIONS_HPP
#define IDL_OPTIONS_HPP

#include "object.hpp"

struct _idl_options : public idl::Object {};

namespace idl {

class Options final : public _idl_options {
public:
    Options() : _outputDir(std::filesystem::current_path().string()) {
        _importDirs.reserve(20);
    }

    bool getDebugMode() const noexcept {
        return _debugMode;
    }

    void setDebugMode(bool enable) noexcept {
        _debugMode = enable;
    }

    bool getWarningsAsErrors() const noexcept {
        return _warningsAsErrors;
    }

    void setWarningsAsErrors(bool enable) noexcept {
        _warningsAsErrors = enable;
    }

    idl_utf8_t getOutputDir() const noexcept {
        return _outputDir.c_str();
    }

    void setOutputDir(idl_utf8_t dir) noexcept {
        _outputDir = std::filesystem::path(dir).make_preferred().string();
    }

    void getImportDirs(idl_uint32_t& dirCount, idl_utf8_t* dirs) const noexcept {
        if (dirs) {
            dirCount = std::min(dirCount, (idl_uint32_t) _importDirs.size());
            for (idl_uint32_t i = 0; i < dirCount; ++i) {
                dirs[i] = _importDirs[i].c_str();
            }
        } else {
            dirCount = (idl_uint32_t) _importDirs.size();
        }
    }

    void setImportDirs(std::span<const idl_utf8_t> dirs) noexcept {
        _importDirs.resize(dirs.size());
        for (size_t i = 0; i < dirs.size(); ++i) {
            _importDirs[i] = dirs[i];
        }
    }

    idl_import_callback_t getImporter(idl_data_t* data) const noexcept {
        if (data) {
            *data = _importerData;
        }
        return _importer;
    }

    void setImporter(idl_import_callback_t callback, idl_data_t data) noexcept {
        _importer     = callback;
        _importerData = data;
    }

    idl_release_import_callback_t getReleaseImport(idl_data_t* data) const noexcept {
        if (data) {
            *data = _releaseImportData;
        }
        return _releaseImport;
    }

    void setReleaseImport(idl_release_import_callback_t callback, idl_data_t data) noexcept {
        _releaseImport     = callback;
        _releaseImportData = data;
    }

    idl_write_callback_t getWriter(idl_data_t* data) const noexcept {
        if (data) {
            *data = _writerData;
        }
        return _writer;
    }

    void setWriter(idl_write_callback_t callback, idl_data_t data) noexcept {
        _writer     = callback;
        _writerData = data;
    }

private:
    bool _debugMode{};
    bool _warningsAsErrors{};
    std::string _outputDir{};
    std::vector<std::string> _importDirs{};
    idl_import_callback_t _importer{};
    idl_data_t _importerData{};
    idl_release_import_callback_t _releaseImport{};
    idl_data_t _releaseImportData{};
    idl_write_callback_t _writer{};
    idl_data_t _writerData{};
};

}; // namespace idl

#endif
