/**
 * @file      idl-options.h
 * @brief     Compiler options.
 * @details   This is where the structures and various compilation options are located.
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @ingroup   files
 * @copyright MIT License
 */
#ifndef IDL_OPTIONS_H
#define IDL_OPTIONS_H

#include "idl-results.h"

IDL_BEGIN

/**
 * @brief   Bool type.
 * @details Boolean ABI represent type.
 * @ingroup enums
 */
typedef enum
{
    IDL_BOOL_TYPE_DEFAULT  = 0, /**< Default type from .idl file. */
    IDL_BOOL_TYPE_INT_32   = 1, /**< 32 bits integer. */
    IDL_BOOL_TYPE_INT_8    = 2, /**< 8 bits integer. */
    IDL_BOOL_TYPE_STD_BOOL = 3, /**< 8 bit C *_Bool* type. */
    IDL_BOOL_TYPE_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} idl_bool_type_t;

/**
 * @brief   Output format files.
 * @details Output default, in single file or multi files.
 * @ingroup enums
 */
typedef enum
{
    IDL_OUTPUT_FILES_DEFAULT  = 0, /**< Default format from .idl file. */
    IDL_OUTPUT_FILES_SINGLE   = 1, /**< Output in single file. */
    IDL_OUTPUT_FILES_MULTI    = 2, /**< Output multi files. */
    IDL_OUTPUT_FILES_MAX_ENUM = 0x7FFFFFFF /**< Max value of enum (not used) */
} idl_output_files_t;

/**
 * @brief   Source code.
 * @details Used to provide source code in memory.
 * @ingroup structs
 */
typedef struct
{
    idl_utf8_t        name; /**< Source name (used to resolve imports). */
    const idl_char_t* data; /**< Source code. */
    idl_uint32_t      size; /**< Size of idl_source_t::data in bytes. */
} idl_source_t;

/**
 * @brief   Api version.
 * @details Used to set ::idl_options_set_version the API version.
 * @ingroup structs
 */
typedef struct
{
    idl_uint32_t major; /**< Major component of the version. */
    idl_uint32_t minor; /**< Minor component of the version. */
    idl_uint32_t micro; /**< Micro component of the version. */
} idl_api_version_t;

/**
 * @brief   Idl Options.
 * @details Specific options for Idl.
 * @ingroup structs
 */
typedef struct
{
    idl_bool_t prefered_original_style; /**< Prefered original code style. */
} idl_idl_options_t;

/**
 * @brief   C Options.
 * @details Specific options for C.
 * @ingroup structs
 */
typedef struct
{
    idl_bool_t add_doc; /**< Add Doxygen documentation. */
    idl_bool_t add_doc_groups; /**< Add Doxygen groups. */
} idl_c_options_t;

/**
 * @name Function pointer types.
 * @brief Function pointers definitions.
 * @{
 */

/**
 * @brief     Callback to get sources.
 * @details   Used to retrieve and compile sources from memory.
 * @param[in] name The name of the file that the compiler is trying to get (for example, when it encounters "import").
 * @param[in] depth Current imports nesting level.
 * @param[in] data User data specified when setting up a callback.
 * @return    Should return the source if the file can be resolved, or null to indicate
 *            to the compiler that it cannot resolve the source and should try to find
 *            the source elsewhere (e.g. via import paths).
 * @sa        If the callback allocates memory, then you can free it in the callback idl_release_import_callback_t.
 * @ingroup   types
 */
typedef idl_source_t*
(*idl_import_callback_t)(idl_utf8_t name,
                         idl_uint32_t depth,
                         idl_data_t data);

/**
 * @brief     Callback to release sources.
 * @details   If idl_import_callback_t allocated memory dynamically for the source, you can free it here.
 * @param[in] source Source for release.
 * @param[in] data User data specified when setting up a callback.
 * @sa        idl_import_callback_t.
 * @ingroup   types
 */
typedef void
(*idl_release_import_callback_t)(idl_source_t* source,
                                 idl_data_t data);

/**
 * @brief     Callback to which the compilation result is passed.
 * @details   If you need to save the compilation result to a location other than the file
 *            system, such as the network or console output, you can use this callback.
 * @param[in] source Source of compiler output.
 * @param[in] data User data specified when setting up a callback.
 * @note      The compiler can output multiple sources. The exact number depends on the selected generator ::idl_generator_t.
 * @ingroup   types
 */
typedef void
(*idl_write_callback_t)(const idl_source_t* source,
                        idl_data_t data);

/** @} */

/**
 * @name Functions of Options.
 * @brief Functions for opaque type ::idl_options_t.
 * @{
 */

/**
 * @brief      Creates new options instance.
 * @details    Creates an object for setting compiler options.
 * @param[out] options New options instance.
 * @return     New options instance.
 * @ingroup    functions
 */
idl_api idl_result_t
idl_options_create(idl_options_t* options);

/**
 * @brief     Increments reference count.
 * @details   Manages options instance lifetime.
 * @param[in] options Target options instance.
 * @return    Reference to same options.
 * @sa        ::idl_options_destroy
 * @ingroup   functions
 */
idl_api idl_options_t
idl_options_reference(idl_options_t options);

/**
 * @brief     Releases options instance.
 * @details   Destroys when reference count reaches zero.
 * @param[in] options Options to destroy.
 * @sa        ::idl_options_reference
 * @ingroup   functions
 */
idl_api void
idl_options_destroy(idl_options_t options);

/**
 * @brief     Get debug mode.
 * @details   Return *TRUE* is debug mode enabled.
 * @param[in] options Target options.
 * @return    *TRUE* is enabled.
 * @sa        ::idl_options_set_debug_mode
 * @ingroup   functions
 */
idl_api idl_bool_t
idl_options_get_debug_mode(idl_options_t options);

/**
 * @brief     Set debug mode.
 * @details   Setting debug compilation output to console.
 * @param[in] options Target options.
 * @param[in] enable Enable debug.
 * @sa        ::idl_options_get_debug_mode
 * @ingroup   functions
 */
idl_api void
idl_options_set_debug_mode(idl_options_t options,
                           idl_bool_t enable);

/**
 * @brief     Get warning handling setting.
 * @details   Return *TRUE* if warnings are treated as errors.
 * @param[in] options Target options.
 * @return    *TRUE* is enabled.
 * @sa        ::idl_options_set_warnings_as_errors
 * @ingroup   functions
 */
idl_api idl_bool_t
idl_options_get_warnings_as_errors(idl_options_t options);

/**
 * @brief     Set warning handling setting.
 * @details   Setting treat warnings as errors.
 * @param[in] options Target options.
 * @param[in] enable Enable treat warnings as errors.
 * @sa        ::idl_options_get_warnings_as_errors
 * @ingroup   functions
 */
idl_api void
idl_options_set_warnings_as_errors(idl_options_t options,
                                   idl_bool_t enable);

/**
 * @brief     Get indents setting.
 * @details   Return space count for generators.
 * @param[in] options Target options.
 * @return    Indents.
 * @sa        ::idl_options_set_indents
 * @ingroup   functions
 */
idl_api idl_uint32_t
idl_options_get_indents(idl_options_t options);

/**
 * @brief     Set indents setting.
 * @details   Setting space count for generators.
 * @param[in] options Target options.
 * @param[in] indents Indent count.
 * @sa        ::idl_options_get_indents
 * @ingroup   functions
 */
idl_api void
idl_options_set_indents(idl_options_t options,
                        idl_uint32_t indents);

/**
 * @brief     Get line length.
 * @details   Return maximum line length for generators.
 * @param[in] options Target options.
 * @return    Maximum length.
 * @sa        ::idl_options_set_line_length
 * @ingroup   functions
 */
idl_api idl_uint32_t
idl_options_get_line_length(idl_options_t options);

/**
 * @brief     Set line length.
 * @details   Setting maximum line length for generators.
 * @param[in] options Target options.
 * @param[in] length Maximum line length.
 * @sa        ::idl_options_get_line_length
 * @ingroup   functions
 */
idl_api void
idl_options_set_line_length(idl_options_t options,
                            idl_uint32_t length);

/**
 * @brief     Get output format.
 * @details   Return output format default, single or multi files.
 * @param[in] options Target options.
 * @return    Output format.
 * @sa        ::idl_options_set_output_files
 * @ingroup   functions
 */
idl_api idl_output_files_t
idl_options_get_output_files(idl_options_t options);

/**
 * @brief     Set output format.
 * @details   Set output default, single or multi files.
 * @param[in] options Target options.
 * @param[in] output Output format.
 * @sa        ::idl_options_get_output_files
 * @ingroup   functions
 */
idl_api void
idl_options_set_output_files(idl_options_t options,
                             idl_output_files_t output);

/**
 * @brief     Get output directory.
 * @details   Returns the path that the compiler will use to save compilation output.
 * @param[in] options Target options.
 * @return    Directory path.
 * @sa        ::idl_options_set_output_dir
 * @ingroup   functions
 */
idl_api idl_utf8_t
idl_options_get_output_dir(idl_options_t options);

/**
 * @brief     Set output directory.
 * @details   Configure the path that the compiler will use to save compilation output.
 * @param[in] options Target options.
 * @param[in] dir Directory path.
 * @note      Compiler output to the file system does not occur if output via a ::idl_options_set_writer is configured.
 * @sa        ::idl_options_get_output_dir
 * @ingroup   functions
 */
idl_api void
idl_options_set_output_dir(idl_options_t options,
                           idl_utf8_t dir);

/**
 * @brief         Returns an array of directories to search for imports.
 * @details       These paths are used to search source code when an import is encountered during compilation.
 * @param[in]     options Target options.
 * @param[in,out] dir_count Number of directories.
 * @param[out]    dirs Import directories.
 * @sa            ::idl_options_set_import_dirs
 * @ingroup       functions
 */
idl_api void
idl_options_get_import_dirs(idl_options_t options,
                            idl_uint32_t* dir_count,
                            idl_utf8_t* dirs);

/**
 * @brief     Configures directories to search for source files.
 * @details   These paths are used to search source code when an import is encountered during compilation.
 * @param[in] options Target options.
 * @param[in] dir_count Number of directories.
 * @param[in] dirs Import directories.
 * @note      These paths are used when resolving imports if the callback passed to ::idl_options_set_importer
 *            did not return a source (if ::idl_options_set_importer was configured)
 * @sa        ::idl_options_get_import_dirs
 * @ingroup   functions
 */
idl_api void
idl_options_set_import_dirs(idl_options_t options,
                            idl_uint32_t dir_count,
                            const idl_utf8_t* dirs);

/**
 * @brief      Get the current import callback.
 * @details    Returns a callback if one has been configured.
 * @param[in]  options Target options.
 * @param[out] data Returning a callback user data pointer (may be null).
 * @return     Returns a callback.
 * @sa         ::idl_options_set_importer
 * @ingroup    functions
 */
idl_api idl_import_callback_t
idl_options_get_importer(idl_options_t options,
                         idl_data_t* data);

/**
 * @brief     Set import callback.
 * @details   Used to resolve code sources, such as when the compiler encounters imports.
 * @param[in] options Target options.
 * @param[in] callback Callback function.
 * @param[in] data Callback user data.
 * @parblock
 * @note      If set, the importer will be used to resolve sources as the highest
 *            priority (then the sources passed to ::idl_compiler_compile in the *sources*
 *            argument will be used, then the directories passed to ::idl_options_set_import_dirs will be used,
 *            and then the current working directory).
 * @endparblock
 * @parblock
 * @note      If *file* was not passed to ::idl_compiler_compile to compile from the file system,
 *            then the importer will also be used to obtain the main (primary) file named *\<input\>*.
 * @endparblock
 * @parblock
 * @note      A typical use of an importer is to read source code from memory.
 * @endparblock
 * @sa        ::idl_options_get_importer
 * @ingroup   functions
 */
idl_api void
idl_options_set_importer(idl_options_t options,
                         idl_import_callback_t callback,
                         idl_data_t data);

/**
 * @brief      Get the current release import callback.
 * @details    Callback for releasing sources allocated via ::idl_options_set_importer.
 * @param[in]  options Target options.
 * @param[out] data Returning a callback user data pointer (may be null).
 * @return     Returns a callback.
 * @sa         ::idl_options_set_release_import
 * @ingroup    functions
 */
idl_api idl_release_import_callback_t
idl_options_get_release_import(idl_options_t options,
                               idl_data_t* data);

/**
 * @brief     Set release import callback.
 * @details   If the callback set in ::idl_options_set_importer allocates data on the heap or creates
 *            any resources, they can be freed by the callback set here.
 * @param[in] options Target options.
 * @param[in] callback Callback function.
 * @param[in] data Callback user data.
 * @sa        ::idl_options_get_release_import
 * @ingroup   functions
 */
idl_api void
idl_options_set_release_import(idl_options_t options,
                               idl_release_import_callback_t callback,
                               idl_data_t data);

/**
 * @brief      Get the current write callback.
 * @details    Returns a callback if one has been configured.
 * @param[in]  options Target options.
 * @param[out] data Returning a callback user data pointer (may be null).
 * @return     Returns a callback.
 * @sa         ::idl_options_set_writer
 * @ingroup    functions
 */
idl_api idl_write_callback_t
idl_options_get_writer(idl_options_t options,
                       idl_data_t* data);

/**
 * @brief     Set write callback.
 * @details   Configures a callback to receive compiler output. If the callback is set, no output
 *            will be made to the file system (::idl_options_set_output_dir will also not be used).
 * @param[in] options Target options.
 * @param[in] callback Callback function.
 * @param[in] data Callback user data.
 * @note      Typical uses of a writer are writing to memory or outputting to the console and the like.
 * @sa        ::idl_options_get_writer
 * @ingroup   functions
 */
idl_api void
idl_options_set_writer(idl_options_t options,
                       idl_write_callback_t callback,
                       idl_data_t data);

/**
 * @brief     Get bool type.
 * @details   Returns the boolean ABI type.
 * @param[in] options Target options.
 * @return    ABI boolean type.
 * @sa        ::idl_options_set_bool_type
 * @ingroup   functions
 */
idl_api idl_bool_type_t
idl_options_get_bool_type(idl_options_t options);

/**
 * @brief     Set bool type.
 * @details   Set boolean ABI type.
 * @param[in] options Target options.
 * @param[in] bool_type Bool type.
 * @sa        ::idl_options_get_bool_type
 * @ingroup   functions
 */
idl_api void
idl_options_set_bool_type(idl_options_t options,
                          idl_bool_type_t bool_type);

/**
 * @brief     Is std types.
 * @details   Returns is used std types.
 * @param[in] options Target options.
 * @return    TRUE is use std types.
 * @sa        ::idl_options_set_std_types
 * @ingroup   functions
 */
idl_api idl_bool_t
idl_options_get_std_types(idl_options_t options);

/**
 * @brief     Set std types.
 * @details   Use stdint.h types directly if TRUE.
 * @param[in] options Target options.
 * @param[in] use_std Use stdint.h types.
 * @sa        ::idl_options_get_std_types
 * @ingroup   functions
 */
idl_api void
idl_options_set_std_types(idl_options_t options,
                          idl_bool_t use_std);

/**
 * @brief     Get Idl options.
 * @details   Returns specific Idl options.
 * @param[in] options Target options.
 * @return    Idl options.
 * @sa        ::idl_options_set_idl_options
 * @ingroup   functions
 */
idl_api idl_idl_options_t
idl_options_get_idl_options(idl_options_t options);

/**
 * @brief     Set Idl options.
 * @details   Set specific Idl options.
 * @param[in] options Target options.
 * @param[in] idl_options Idl options.
 * @sa        ::idl_options_get_idl_options
 * @ingroup   functions
 */
idl_api void
idl_options_set_idl_options(idl_options_t options,
                            const idl_idl_options_t* idl_options);

/**
 * @brief     Get C options.
 * @details   Returns specific C options.
 * @param[in] options Target options.
 * @return    C options.
 * @sa        ::idl_options_set_c_options
 * @ingroup   functions
 */
idl_api idl_c_options_t
idl_options_get_c_options(idl_options_t options);

/**
 * @brief     Set C options.
 * @details   Set specific C options.
 * @param[in] options Target options.
 * @param[in] coptions C options.
 * @sa        ::idl_options_get_c_options
 * @ingroup   functions
 */
idl_api void
idl_options_set_c_options(idl_options_t options,
                          const idl_c_options_t* coptions);

/**
 * @brief     Get api version.
 * @details   Returns the API version or null.
 * @param[in] options Target options.
 * @return    API version or null.
 * @sa        ::idl_options_set_version
 * @ingroup   functions
 */
idl_api const idl_api_version_t*
idl_options_get_version(idl_options_t options);

/**
 * @brief     Set api version.
 * @details   Sets the API version that will be saved in the compiler output.
 * @param[in] options Target options.
 * @param[in] version Api version.
 * @note      If not set, then the API version will be taken from the `[version(major,minor,micro)]`
 *            attribute (sample: `api Sample [version(2,3,1)]`). If the api does not have a version
 *            attribute specified, then the version will be taken as `0.0.0`.
 * @sa        ::idl_options_get_version
 * @ingroup   functions
 */
idl_api void
idl_options_set_version(idl_options_t options,
                        const idl_api_version_t* version);

/** @} */

IDL_END

#endif /* IDL_OPTIONS_H */
