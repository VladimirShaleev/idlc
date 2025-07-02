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
 * @details    TODO:
 * @param[out] options New options instance.
 * @return     TODO:
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
 * @details   Enable debug mode.
 * @param[in] options Target options.
 * @param[in] enable Enable debug.
 * @sa        ::idl_options_get_debug_mode
 */
idl_api void
idl_options_set_debug_mode(idl_options_t options,
                           idl_bool_t enable);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] options Target options.
 * @return    TODO.
 * @sa        ::idl_options_set_warnings_as_errors
 */
idl_api idl_bool_t
idl_options_get_warnings_as_errors(idl_options_t options);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] options Target options.
 * @param[in] enable TODO.
 * @sa        ::idl_options_get_warnings_as_errors
 */
idl_api void
idl_options_set_warnings_as_errors(idl_options_t options,
                                   idl_bool_t enable);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] options Target options.
 * @return    TODO.
 * @sa        ::idl_options_set_output_dir
 */
idl_api idl_utf8_t
idl_options_get_output_dir(idl_options_t options);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] options Target options.
 * @param[in] dir TODO.
 * @sa        ::idl_options_get_output_dir
 */
idl_api void
idl_options_set_output_dir(idl_options_t options,
                           idl_utf8_t dir);

/**
 * @brief         TODO.
 * @details       TODO.
 * @param[in]     options Target options.
 * @param[in,out] dir_count Number of directories.
 * @param[out]    dirs Import directories.
 * @return        TODO.
 * @sa            ::idl_options_set_import_dirs
 */
idl_api void
idl_options_get_import_dirs(idl_options_t options,
                            idl_uint32_t* dir_count,
                            idl_utf8_t* dirs);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] options Target options.
 * @param[in] dir_count Number of directories.
 * @param[in] dirs Import directories.
 * @sa        ::idl_options_get_import_dirs
 */
idl_api void
idl_options_set_import_dirs(idl_options_t options,
                            idl_uint32_t dir_count,
                            const idl_utf8_t* dirs);

/**
 * @brief      TODO.
 * @details    TODO.
 * @param[in]  options Target options.
 * @param[out] data Callback user data pointer.
 * @return     TODO.
 */
idl_api idl_import_callback_t
idl_options_get_importer(idl_options_t options,
                         idl_data_t* data);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] options Target options.
 * @param[in] callback Callback function.
 * @param[in] data Callback user data.
 */
idl_api void
idl_options_set_importer(idl_options_t options,
                         idl_import_callback_t callback,
                         idl_data_t data);

/**
 * @brief      TODO.
 * @details    TODO.
 * @param[in]  options Target options.
 * @param[out] data Callback user data pointer.
 * @return     TODO.
 */
idl_api idl_release_import_callback_t
idl_options_get_release_import(idl_options_t options,
                               idl_data_t* data);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] options Target options.
 * @param[in] callback Callback function.
 * @param[in] data Callback user data.
 */
idl_api void
idl_options_set_release_import(idl_options_t options,
                               idl_release_import_callback_t callback,
                               idl_data_t data);

/**
 * @brief      TODO.
 * @details    TODO.
 * @param[in]  options Target options.
 * @param[out] data Callback user data pointer.
 * @return     TODO.
 */
idl_api idl_write_callback_t
idl_options_get_writer(idl_options_t options,
                       idl_data_t* data);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] options Target options.
 * @param[in] callback Callback function.
 * @param[in] data Callback user data.
 */
idl_api void
idl_options_set_writer(idl_options_t options,
                       idl_write_callback_t callback,
                       idl_data_t data);

/**
 * @brief         TODO.
 * @details       TODO.
 * @param[in]     options Target options.
 * @param[in,out] addition_count Number of additions.
 * @param[out]    additions Additions.
 * @return        TODO.
 * @sa            ::idl_options_set_additions
 */
idl_api void
idl_options_get_additions(idl_options_t options,
                          idl_uint32_t* addition_count,
                          idl_utf8_t* additions);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] options Target options.
 * @param[in] addition_count Number of additions.
 * @param[in] additions Additions.
 * @note      Generator specific. For C these are additional headers.
 * @sa        ::idl_options_get_additions
 */
idl_api void
idl_options_set_additions(idl_options_t options,
                          idl_uint32_t addition_count,
                          const idl_utf8_t* additions);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] options Target options.
 * @sa        ::idl_options_set_version
 */
idl_api const idl_api_version_t*
idl_options_get_version(idl_options_t options);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] options Target options.
 * @param[in] version TODO.
 * @sa        ::idl_options_get_version
 */
idl_api void
idl_options_set_version(idl_options_t options,
                        const idl_api_version_t* version);

IDL_END

#endif /* IDL_OPTIONS_H */
