/**
 * @file      idl-options.h
 * @brief     TODO.
 * @details   TODO.
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @copyright MIT License
 */
#ifndef IDL_OPTIONS_H
#define IDL_OPTIONS_H

#include "idl-results.h"

IDL_BEGIN

/**
 * @brief   Source code.
 * @details Used to provide source code in memory.
 */
typedef struct
{
    idl_utf8_t   name; /**< Source name (used to resolve imports) */
    idl_cdata_t  data; /**< Source code. */
    idl_uint32_t size; /**< Size of idl_source_t::data in bytes. */
} idl_source_t;

/**
 * @brief   Api version.
 * @details Used to set ::idl_options_set_version the API version.
 */
typedef struct
{
    idl_uint32_t major; /**< Major component of the version. */
    idl_uint32_t minor; /**< Minor component of the version. */
    idl_uint32_t micro; /**< Micro component of the version. */
} idl_api_version_t;

/**
 * @brief     Callback to get sources.
 * @details   Used to retrieve and compile sources from memory.
 * @param[in] name The name of the file that the compiler is trying to get (for example, when it encounters "import").
 * @param[in] depth Current imports nesting level.
 * @param[in] data User data specified when setting up a callback.
 * @return    Should return the source if the file can be resolved, or null to indicate
 *            to the compiler that it cannot resolve the source and should try to find
 *            the source elsewhere (e.g. via import paths)
 * @sa        If the callback allocates memory, then you can free it in the callback idl_release_import_callback_t.
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
 * @note      The compiler can output multiple sources. The exact number depends on the selected generator idl_generator_t.
 */
typedef void
(*idl_write_callback_t)(const idl_source_t* source,
                        idl_data_t data);

/**
 * @brief      Creates new options instance.
 * @details    Creates an object for setting compiler options.
 * @param[out] options New options instance.
 * @return     New options instance.
 */
idl_api idl_result_t
idl_options_create(idl_options_t* options);

/**
 * @brief     Increments reference count.
 * @details   Manages options instance lifetime.
 * @param[in] options Target options instance.
 * @return    Reference to same options.
 * @sa        ::idl_options_destroy
 */
idl_api idl_options_t
idl_options_reference(idl_options_t options);

/**
 * @brief     Releases options instance.
 * @details   Destroys when reference count reaches zero.
 * @param[in] options Options to destroy.
 * @sa        ::idl_options_reference
 */
idl_api void
idl_options_destroy(idl_options_t options);

/**
 * @brief     Get debug mode.
 * @details   Return *TRUE* is debug mode enabled.
 * @param[in] options Target options.
 * @return    *TRUE* is enabled.
 * @sa        ::idl_options_set_debug_mode
 */
idl_api idl_bool_t
idl_options_get_debug_mode(idl_options_t options);

/**
 * @brief     Set debug mode.
 * @details   Setting debug compilation output to console.
 * @param[in] options Target options.
 * @param[in] enable Enable debug.
 * @sa        ::idl_options_get_debug_mode
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
 */
idl_api idl_bool_t
idl_options_get_warnings_as_errors(idl_options_t options);

/**
 * @brief     Set warning handling setting.
 * @details   Setting treat warnings as errors.
 * @param[in] options Target options.
 * @param[in] enable Enable treat warnings as errors.
 * @sa        ::idl_options_get_warnings_as_errors
 */
idl_api void
idl_options_set_warnings_as_errors(idl_options_t options,
                                   idl_bool_t enable);

/**
 * @brief     Get output directory.
 * @details   Returns the path that the compiler will use to save compilation output.
 * @param[in] options Target options.
 * @return    Directory path.
 * @sa        ::idl_options_set_output_dir
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
 * @return        Array of directories paths.
 * @sa            ::idl_options_set_import_dirs
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
 *            priority (then the directories passed to ::idl_options_set_import_dirs will be used,
 *            and then the current working directory).
 * @endparblock
 * @parblock
 * @note      If *file* was not passed to ::idl_compiler_compile to compile from the file system,
 *            then the importer will also be used to obtain the main (primary) file named *<input>*.
 * @endparblock
 * @parblock
 * @note      A typical use of an importer is to read source code from memory.
 * @endparblock
 * @sa        ::idl_options_get_importer
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
 */
idl_api void
idl_options_set_writer(idl_options_t options,
                       idl_write_callback_t callback,
                       idl_data_t data);

/**
 * @brief         Get additional parameters.
 * @details       Returns an array of additional parameters.
 * @param[in]     options Target options.
 * @param[in,out] addition_count Number of additions.
 * @param[out]    additions Additions.
 * @return        Parameter array.
 * @sa            ::idl_options_set_additions
 */
idl_api void
idl_options_get_additions(idl_options_t options,
                          idl_uint32_t* addition_count,
                          idl_utf8_t* additions);

/**
 * @brief     Set additional parameters.
 * @details   Sets additional parameters specific to the generator (idl_generator_t).
 * @param[in] options Target options.
 * @param[in] addition_count Number of additions.
 * @param[in] additions Additions.
 * @note      Supported Generators:
 *            - ::IDL_GENERATOR_C - additional headers included in the API header file;
 *            - ::IDL_GENERATOR_JS - no specific parameters.
 *            
 * @sa        ::idl_options_get_additions
 */
idl_api void
idl_options_set_additions(idl_options_t options,
                          idl_uint32_t addition_count,
                          const idl_utf8_t* additions);

/**
 * @brief     Get api version.
 * @details   Returns the API version or null.
 * @param[in] options Target options.
 * @sa        ::idl_options_set_version
 */
idl_api const idl_api_version_t*
idl_options_get_version(idl_options_t options);

/**
 * @brief     Set api version.
 * @details   Sets the API version that will be saved in the compiler output.
 * @param[in] options Target options.
 * @param[in] version TODO.
 * @note      If not set, then the API version will be taken from the [version(major,minor,micro)]
 *            attribute (sample: api Sample [version(2,3,1)]). If the api does not have a version
 *            attribute specified, then the version will be taken as 0.0.0.
 * @sa        ::idl_options_get_version
 */
idl_api void
idl_options_set_version(idl_options_t options,
                        const idl_api_version_t* version);

IDL_END

#endif /* IDL_OPTIONS_H */
