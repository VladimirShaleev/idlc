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
 * @details Description of the source code.
 */
typedef struct
{
    idl_utf8_t   name; /**< Name */
    idl_utf8_t   data; /**< The source code is passed in case of reading from memory, otherwise set to null. */
    idl_uint32_t size; /**< Size of idl_source_t::data in bytes. */
} idl_source_t;

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] name TODO.
 * @param[in] depth TODO.
 * @param[in] data TODO.
 */
typedef idl_source_t*
(*idl_import_callback_t)(idl_utf8_t name,
                         idl_uint32_t depth,
                         idl_data_t data);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] source TODO.
 * @param[in] data TODO.
 */
typedef void
(*idl_release_import_callback_t)(idl_source_t* source,
                                 idl_data_t data);

/**
 * @brief     TODO.
 * @details   TODO.
 * @param[in] source TODO.
 * @param[in] data TODO.
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

IDL_END

#endif /* IDL_OPTIONS_H */
