#ifndef CASE_CONVERTER_HPP
#define CASE_CONVERTER_HPP

#include <algorithm>
#include <cassert>
#include <sstream>
#include <vector>

enum struct Case {
    LispCase,
    ScreamingLispCase,
    CamelCase,
    PascalCase,
    SnakeCase,
    ScreamingSnakeCase
};

inline std::string& lower(std::string& str) {
    std::transform(str.cbegin(), str.cend(), str.begin(), [](auto c) {
        return std::tolower(c);
    });
    return str;
}

inline std::string& upper(std::string& str) {
    std::transform(str.cbegin(), str.cend(), str.begin(), [](auto c) {
        return std::toupper(c);
    });
    return str;
}

inline std::vector<std::string> tokenize(const std::string& str) {
    std::vector<std::string> tokens;
    std::ostringstream ss;
    char prevC = '\0';
    for (auto c : str) {
        if ((std::isupper(c) && !std::isupper(prevC)) || (std::isdigit(c) && !std::isdigit(prevC)) ||
            (!std::isdigit(c) && std::isdigit(prevC))) {
            auto token = ss.str();
            if (token.length() > 0) {
                tokens.push_back(token);
            }
            ss = std::ostringstream();
        }
        prevC = c;
        ss << c;
    }
    tokens.push_back(ss.str());
    return tokens;
}

inline std::string convert(const std::string& str, Case caseConvention) {
    auto tokens = tokenize(str);

    char prevSymbol = '\0';
    std::ostringstream ss;

    switch (caseConvention) {
        case Case::LispCase:
            for (size_t i = 0; i < tokens.size(); ++i) {
                ss << lower(tokens[i]);
                if (i + 1 < tokens.size()) {
                    ss << '-';
                }
            }
            break;

        case Case::ScreamingLispCase:
            for (size_t i = 0; i < tokens.size(); ++i) {
                ss << upper(tokens[i]);
                if (i + 1 < tokens.size()) {
                    ss << '-';
                }
            }
            break;

        case Case::CamelCase:
            for (size_t i = 0; i < tokens.size(); ++i) {
                auto& token = tokens[i];
                if (i == 0) {
                    token[0] = std::tolower(token[0]);
                }
                ss << token;
            }
            break;

        case Case::PascalCase:
            for (auto& token : tokens) {
                ss << token;
            }
            break;

        case Case::SnakeCase:
            for (size_t i = 0; i < tokens.size(); ++i) {
                ss << lower(tokens[i]);
                if (i + 1 < tokens.size()) {
                    ss << '_';
                }
            }
            break;

        case Case::ScreamingSnakeCase:
            for (size_t i = 0; i < tokens.size(); ++i) {
                ss << upper(tokens[i]);
                if (i + 1 < tokens.size()) {
                    ss << '_';
                }
            }
            break;

        default:
            assert(!"unreachable code");
            break;
    }
    return ss.str();
}

#endif
