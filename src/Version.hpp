#ifndef VERSION_HPP
#define VERSION_HPP

#define IDLC_VERSION_MAJOR 1
#define IDLC_VERSION_MINOR 0
#define IDLC_VERSION_MICRO 0

#define IDLC_VERSION_ENCODE(major, minor, micro) (((unsigned long) major) << 16 | (minor) << 8 | (micro))

#define IDLC_VERSION_STRINGIZE_(major, minor, micro) #major "." #minor "." #micro
#define IDLC_VERSION_STRINGIZE(major, minor, micro)  IDLC_VERSION_STRINGIZE_(major, minor, micro)

#define IDLC_VERSION IDLC_VERSION_ENCODE( \
    IDLC_VERSION_MAJOR, \
    IDLC_VERSION_MINOR, \
    IDLC_VERSION_MICRO)

#define IDLC_VERSION_STRING IDLC_VERSION_STRINGIZE( \
    IDLC_VERSION_MAJOR, \
    IDLC_VERSION_MINOR, \
    IDLC_VERSION_MICRO)

#endif
