#pragma once

#include "TSBase.h"
#include "TSString.h"
#include <regex>
#include <string>
#include <stdexcept>

class TC_GAME_API TSRegExp
{
private:
    std::regex m_regex;
    std::string m_pattern;
    std::string m_flags;
    bool m_global = false;
    bool m_ignoreCase = false;
    bool m_multiline = false;
    bool m_dotall = false;

    // std::regex (ECMAScript flavor) does not support a dotall mode natively,
    // so when the `s` flag is requested we rewrite the pattern: every dot that
    // isn't escaped or inside a character class becomes `[\s\S]`, which DOES
    // match every character including newlines. Patterns are preprocessed once
    // at construction.
    static std::string applyDotall(const std::string& pattern)
    {
        std::string result;
        result.reserve(pattern.size());
        bool escaped = false;
        bool inCharClass = false;
        for (char c : pattern)
        {
            if (escaped)
            {
                result += c;
                escaped = false;
                continue;
            }
            if (c == '\\')
            {
                result += c;
                escaped = true;
                continue;
            }
            if (c == '[' && !inCharClass)
            {
                inCharClass = true;
                result += c;
                continue;
            }
            if (c == ']' && inCharClass)
            {
                inCharClass = false;
                result += c;
                continue;
            }
            if (c == '.' && !inCharClass)
            {
                result += "[\\s\\S]";
                continue;
            }
            result += c;
        }
        return result;
    }

    std::regex_constants::syntax_option_type parseFlags(const std::string& flags)
    {
        auto options = std::regex_constants::ECMAScript;
        for (char flag : flags)
        {
            switch (flag)
            {
                case 'i':
                    m_ignoreCase = true;
                    options |= std::regex_constants::icase;
                    break;
                case 'g':
                    // JS-style global semantics need lastIndex tracking which
                    // we don't model here; record the flag so callers can see
                    // it but otherwise treat each call as a fresh search.
                    m_global = true;
                    break;
                case 'm':
                    m_multiline = true;
                    options |= std::regex_constants::multiline;
                    break;
                case 's':
                    m_dotall = true;
                    break;
            }
        }
        return options;
    }

public:
    TSRegExp(const std::string& pattern)
        : m_pattern(pattern)
        , m_flags("")
    {
        try {
            m_regex = std::regex(pattern, std::regex_constants::ECMAScript);
        } catch (const std::regex_error& e) {
            throw std::runtime_error(std::string("Invalid regular expression /") + pattern + "/: " + e.what());
        }
    }

    TSRegExp(const std::string& pattern, const std::string& flags)
        : m_pattern(pattern)
        , m_flags(flags)
    {
        try {
            const auto options = parseFlags(flags);
            const std::string effective = m_dotall ? applyDotall(pattern) : pattern;
            m_regex = std::regex(effective, options);
        } catch (const std::regex_error& e) {
            throw std::runtime_error(std::string("Invalid regular expression /") + pattern + "/" + flags + ": " + e.what());
        }
    }

    bool test(const std::string& str) const
    {
        return std::regex_search(str, m_regex);
    }

    // Returns the first matched substring, or an empty string if there is no
    // match. Capture groups are not yet exposed; this is intentionally a
    // simpler shape than JS RegExp.prototype.exec().
    std::string exec(const std::string& str) const
    {
        std::smatch match;
        if (std::regex_search(str, match, m_regex))
        {
            return match[0].str();
        }
        return "";
    }

    std::string source() const { return m_pattern; }

    bool global() const { return m_global; }
    bool ignoreCase() const { return m_ignoreCase; }
    bool multiline() const { return m_multiline; }
    bool dotall() const { return m_dotall; }

    std::string toString() const
    {
        return "/" + m_pattern + "/" + m_flags;
    }
};

// Mirror the JS `new RegExp(pattern, flags?)` constructor shape from livescript.
inline TSRegExp RegExp(const std::string& pattern)
{
    return TSRegExp(pattern);
}

inline TSRegExp RegExp(const std::string& pattern, const std::string& flags)
{
    return TSRegExp(pattern, flags);
}
