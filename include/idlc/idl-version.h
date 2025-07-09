/**
 * @file      idl-version.h
 * @brief     Library version information and utilities.
 * @details   This header provides version information for the Idl library,
 *            including version number components and macros for version comparison
 *            and string generation. It supports:
 *            - Major/Minor/Micro version components
 *            - Integer version encoding
 *            - String version generation
 *            
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @ingroup   files
 * @copyright MIT License
 */
#ifndef IDL_VERSION_H
#define IDL_VERSION_H

/**
 * @name  Version Components.
 * @brief Individual components of the library version.
 * @{
 */

/**
 * @brief   Major version number (API-breaking changes).
 * @sa      IDL_VERSION
 * @sa      IDL_VERSION_STRING
 * @ingroup macros
 */
#define IDL_VERSION_MAJOR 1

/**
 * @brief   Minor version number (backwards-compatible additions).
 * @sa      IDL_VERSION
 * @sa      IDL_VERSION_STRING
 * @ingroup macros
 */
#define IDL_VERSION_MINOR 5

/**
 * @brief   Micro version number (bug fixes and patches).
 * @sa      IDL_VERSION
 * @sa      IDL_VERSION_STRING
 * @ingroup macros
 */
#define IDL_VERSION_MICRO 11

/** @} */

/**
 * @name  Version Utilities.
 * @brief Macros for working with version numbers.
 * @{
 */

/**
 * @brief     Encodes version components into a single integer.
 * @details   Combines major, minor, and micro versions into a 32-bit value:
 *            - Bits 24-31: Major version
 *            - Bits 16-23: Minor version
 *            - Bits 0-15: Micro version
 * @param[in] major Major version number.
 * @param[in] minor Minor version number.
 * @param[in] micro Micro version number.
 * @return    Encoded version as unsigned long.
 * @sa        IDL_VERSION
 * @ingroup   macros
 */
#define IDL_VERSION_ENCODE(major, minor, micro) (((unsigned long) major) << 16 | (minor) << 8 | (micro))

/**
 * @brief     Internal macro for string version generation.
 * @details   Helper macro that stringizes version components (e.g., 1, 5, 11 -> "1.5.11").
 * @param[in] major Major version number.
 * @param[in] minor Minor version number.
 * @param[in] micro Micro version number.
 * @return    Stringified version.
 * @note      For internal use only.
 * @ingroup   macros
 * @private
 */
#define IDL_VERSION_STRINGIZE_(major, minor, micro) #major "." #minor "." #micro

/**
 * @brief     Creates version string from components.
 * @details   Generates a string literal from version components (e.g., 1, 5, 11 -> "1.5.11").
 * @param[in] major Major version number.
 * @param[in] minor Minor version number.
 * @param[in] micro Micro version number.
 * @return    Stringified version.
 * @sa        IDL_VERSION_STRING
 * @ingroup   macros
 */
#define IDL_VERSION_STRINGIZE(major, minor, micro)  IDL_VERSION_STRINGIZE_(major, minor, micro)

/** @} */

/**
 * @name  Current Version.
 * @brief Macros representing the current library version.
 * @{
 */

/**
 * @brief   Encoded library version as integer.
 * @details Combined version value suitable for numeric comparisons.
 *          Use #IDL_VERSION_STRING for human-readable format.
 * @sa      IDL_VERSION_STRING
 * @ingroup macros
 */
#define IDL_VERSION IDL_VERSION_ENCODE( \
    IDL_VERSION_MAJOR, \
    IDL_VERSION_MINOR, \
    IDL_VERSION_MICRO)

/**
 * @brief   Library version as human-readable string.
 * @details Version string in "MAJOR.MINOR.MICRO" format (e.g., "1.5.11").
 *          Use #IDL_VERSION for numeric comparisons.
 * @sa      IDL_VERSION
 * @ingroup macros
 */
#define IDL_VERSION_STRING IDL_VERSION_STRINGIZE( \
    IDL_VERSION_MAJOR, \
    IDL_VERSION_MINOR, \
    IDL_VERSION_MICRO)

/** @} */

#endif /* IDL_VERSION_H */
