/**
 * @file      sample-version.h
 * @brief     Library version information and utilities.
 * @details   This header provides version information for the Sample library,
 *            including version number components and macros for version comparison
 *            and string generation. It supports:
 *            - Major/Minor/Micro version components
 *            - Integer version encoding
 *            - String version generation
 *            
 * @author    Author <author@email.org>
 * @copyright MIT License
 */
#ifndef SAMPLE_VERSION_H
#define SAMPLE_VERSION_H

/**
 * @name  Version Components.
 * @brief Individual components of the library version.
 * @{
 */

/**
 * @brief Major version number (API-breaking changes).
 * @sa    SAMPLE_VERSION
 * @sa    SAMPLE_VERSION_STRING
 */
#define SAMPLE_VERSION_MAJOR 1

/**
 * @brief Minor version number (backwards-compatible additions).
 * @sa    SAMPLE_VERSION
 * @sa    SAMPLE_VERSION_STRING
 */
#define SAMPLE_VERSION_MINOR 0

/**
 * @brief Micro version number (bug fixes and patches).
 * @sa    SAMPLE_VERSION
 * @sa    SAMPLE_VERSION_STRING
 */
#define SAMPLE_VERSION_MICRO 0

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
 * @sa        SAMPLE_VERSION
 */
#define SAMPLE_VERSION_ENCODE(major, minor, micro) (((unsigned long) major) << 16 | (minor) << 8 | (micro))

/**
 * @brief     Internal macro for string version generation.
 * @details   Helper macro that stringizes version components (e.g., 1, 0, 0 -> "1.0.0").
 * @param[in] major Major version number.
 * @param[in] minor Minor version number.
 * @param[in] micro Micro version number.
 * @return    Stringified version.
 * @note      For internal use only.
 * @private
 */
#define SAMPLE_VERSION_STRINGIZE_(major, minor, micro) #major "." #minor "." #micro

/**
 * @brief     Creates version string from components.
 * @details   Generates a string literal from version components (e.g., 1, 0, 0 -> "1.0.0").
 * @param[in] major Major version number.
 * @param[in] minor Minor version number.
 * @param[in] micro Micro version number.
 * @return    Stringified version.
 * @sa        SAMPLE_VERSION_STRING
 */
#define SAMPLE_VERSION_STRINGIZE(major, minor, micro)  SAMPLE_VERSION_STRINGIZE_(major, minor, micro)

/** @} */

/**
 * @name  Current Version.
 * @brief Macros representing the current library version.
 * @{
 */

/**
 * @brief   Encoded library version as integer.
 * @details Combined version value suitable for numeric comparisons.
 *          Use #SAMPLE_VERSION_STRING for human-readable format.
 * @sa      SAMPLE_VERSION_STRING
 */
#define SAMPLE_VERSION SAMPLE_VERSION_ENCODE( \
    SAMPLE_VERSION_MAJOR, \
    SAMPLE_VERSION_MINOR, \
    SAMPLE_VERSION_MICRO)

/**
 * @brief   Library version as human-readable string.
 * @details Version string in "MAJOR.MINOR.MICRO" format (e.g., "1.0.0").
 *          Use #SAMPLE_VERSION for numeric comparisons.
 * @sa      SAMPLE_VERSION
 */
#define SAMPLE_VERSION_STRING SAMPLE_VERSION_STRINGIZE( \
    SAMPLE_VERSION_MAJOR, \
    SAMPLE_VERSION_MINOR, \
    SAMPLE_VERSION_MICRO)

/** @} */

#endif /* SAMPLE_VERSION_H */
