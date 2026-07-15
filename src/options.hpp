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

    idl_uint32_t getIndents() const noexcept {
        return _indents;
    }

    void setIndents(idl_uint32_t indents) noexcept {
        _indents = std::min(indents, idl_uint32_t(64u));
    }

    idl_uint32_t getLineLength() const noexcept {
        return _lineLength;
    }

    void setLineLength(idl_uint32_t length) noexcept {
        _lineLength = std::clamp(length, idl_uint32_t(60u), idl_uint32_t(10000u));
    }

    idl_output_files_t getOutputFiles() const noexcept {
        return _outputFiles;
    }

    void setOutputFiles(idl_output_files_t output) noexcept {
        _outputFiles = output;
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

    idl_bool_type_t getBoolType() const noexcept {
        return _boolType;
    }

    void setBoolType(idl_bool_type_t boolType) noexcept {
        _boolType = boolType;
    }

    idl_idl_options_t getIdlOptions() const noexcept {
        return _idlOptions;
    }

    void setIdlOptions(const idl_idl_options_t* idl_options) noexcept {
        _idlOptions = idl_options ? *idl_options : idl_idl_options_t{};
    }

    idl_c_options_t getCOptions() const noexcept {
        return _cOptions;
    }

    void setCOptions(const idl_c_options_t* coptions) noexcept {
        _cOptions = coptions ? *coptions : idl_c_options_t{};
    }

    const idl_api_version_t* getVersion() const noexcept {
        return _version.has_value() ? &_version.value() : nullptr;
    }

    void setVersion(const idl_api_version_t* version) noexcept {
        _version = version ? std::make_optional(*version) : std::nullopt;
    }

private:
    bool _debugMode{};
    bool _warningsAsErrors{};
    uint32_t _indents{ 4 };
    uint32_t _lineLength{ 120 };
    idl_output_files_t _outputFiles{};
    std::string _outputDir{};
    std::vector<std::string> _importDirs{};
    std::vector<std::string> _additions{};
    idl_bool_type_t _boolType{};
    idl_idl_options_t _idlOptions{};
    idl_c_options_t _cOptions{};
    idl_import_callback_t _importer{};
    idl_data_t _importerData{};
    idl_release_import_callback_t _releaseImport{};
    idl_data_t _releaseImportData{};
    idl_write_callback_t _writer{};
    idl_data_t _writerData{};
    std::optional<idl_api_version_t> _version{};
};

}; // namespace idl

#endif
