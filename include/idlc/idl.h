/**
 * @file      idl.h
 * @brief     IDLC
 * @details   IDLC - Interface Definition Language Compiler. It is an abstract API
 *            description language for platform- and language-independent interaction
 *            with the implemented interface.
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @ingroup   files
 * @copyright MIT License
 *
 *     MIT License
 *     
 *     Copyright (c) 2025 Vladimir Shaleev
 *     
 *     Permission is hereby granted, free of charge, to any person obtaining a copy
 *     of this software and associated documentation files (the "Software"), to deal
 *     in the Software without restriction, including without limitation the rights
 *     to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *     copies of the Software, and to permit persons to whom the Software is
 *     furnished to do so, subject to the following conditions:
 *     
 *     The above copyright notice and this permission notice shall be included in all
 *     copies or substantial portions of the Software.
 *     
 *     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *     IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *     FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *     AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *     LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *     OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *     SOFTWARE.
 *     
 *     Contributor(s):
 *        Vladimir Shaleev <vladimirshaleev@gmail.com>
 */
#ifndef IDL_H
#define IDL_H

#include "idl-options.h"

/**
 * @brief   Generation language
 * @note    Enumeration possible languages for generating interfaces and wrapping C libraries for other languages.
 * @ingroup enums
 */
typedef enum
{
    IDL_GENERATOR_C           = 0, /**< C generator */
    IDL_GENERATOR_JAVA_SCRIPT = 3, /**< JavaScript generator (generates Embind bindings) */
    IDL_GENERATOR_MAX_ENUM    = 0x7FFFFFFF /**< Max value of enum (not used) */
} idl_generator_t;

/**
 * @brief   Current library version as packed 32-bit value.
 * @details Format: (major << 16) | (minor << 8) | micro.
 * @return  Return packed version number
 * @ingroup functions
 */
idl_api idl_uint32_t
idl_version(void);

/**
 * @brief   Current library version as human-readable string.
 * @details Format: "major.minor.micro", eg: "1.3.0".
 * @return  Return version string.
 * @ingroup functions
 */
idl_api idl_utf8_t
idl_version_string(void);

/**
 * @name Functions of Compiler.
 * @brief Functions for opaque type ::idl_compiler_t.
 * @{
 */

/**
 * @brief      Creates new compiler instance.
 * @details    Creates an object for IDL compilation.
 * @param[out] compiler New compiler instance.
 * @return     New compiler instance
 * @ingroup    functions
 */
idl_api idl_result_t
idl_compiler_create(idl_compiler_t* compiler);

/**
 * @brief     Increments reference count.
 * @details   Manages compiler instance lifetime.
 * @param[in] compiler Target compiler instance.
 * @return    Reference to same compiler.
 * @sa        ::idl_compiler_destroy
 * @ingroup   functions
 */
idl_api idl_compiler_t
idl_compiler_reference(idl_compiler_t compiler);

/**
 * @brief     Releases compiler instance.
 * @details   Destroys when reference count reaches zero.
 * @param[in] compiler Compiler to destroy.
 * @sa        ::idl_compiler_reference
 * @ingroup   functions
 */
idl_api void
idl_compiler_destroy(idl_compiler_t compiler);

/**
 * @brief      Compile IDL.
 * @param[in]  compiler Target compiler.
 * @param[in]  generator Target of generator.
 * @param[in]  file Path to .idl file for compile.
 * @param[in]  source_count Number of sources.
 * @param[in]  sources Sources.
 * @param[in]  options Compile options, may be null.
 * @param[out] result Compilation result.
 * @return     Compilation result.
 * @parblock
 * @note       To read source code from memory instead of the file system, use *sources* and/or configure
 *             the importer with ::idl_options_set_importer and pass the *file* argument as empty.
 * @endparblock
 * @parblock
 * @note       Priorities for resolving source code imports:
 *             - ::idl_options_set_importer - import callback if specified;
 *             - *sources* - then the source code array, if specified;
 *             - ::idl_options_set_import_dirs - then in the paths to the import directories, if specified;
 *             - then the current working directory.
 *             
 * @endparblock
 * @ingroup    functions
 */
idl_api idl_result_t
idl_compiler_compile(idl_compiler_t compiler,
                     idl_generator_t generator,
                     idl_utf8_t file,
                     idl_uint32_t source_count,
                     const idl_source_t* sources,
                     idl_options_t options,
                     idl_compilation_result_t* result);

/** @} */

#endif /* IDL_H */
