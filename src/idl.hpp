#ifndef IDL_HPP
#define IDL_HPP

#ifndef IDL_STATIC_BUILD
# ifdef _MSC_VER
#  define idl_api __declspec(dllexport)
# elif __GNUC__ >= 4
#  define idl_api __attribute__((visibility("default")))
# else
#  define idl_api
# endif
#endif

#include "idlc/idl.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <span>
#include <sstream>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include <fmt/base.h>
#include <fmt/ostream.h>
#include <magic_enum/magic_enum.hpp>
#include <xxhash.h>

#endif
