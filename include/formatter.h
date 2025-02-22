#pragma once
#include "ctmap.h"

#include <format>
#include <iomanip>
#include <sstream>


template<ctmap::TagMap _TagMap>
struct std::formatter<_TagMap, char>
{
    bool multiline = false;

    template<typename _Context>
    constexpr auto parse(_Context& context)
    {
        auto it = context.begin();
        if (it == context.end())
            return it;
        if (*it == 'm')
        {
            multiline = true;
            ++it;
        }
        if (it != context.end() && *it != '}')
            throw std::format_error("Invalid format args for tag_map.");
        return it;
    }

    template<class _Context>
    auto format(_TagMap const& tagMap,
                _Context& context) const
    {
        std::ostringstream out;
        char const* delim = multiline ? "\n    " : ", ";
        out << '{' << (multiline ? delim : " ");
        bool skipDelim = true;
        auto const print = [&](auto const& taggedType)
        {
            if (!skipDelim)
                out << delim;
            skipDelim = false;
            out << std::quoted(taggedType.tag.value) << ": " << std::quoted((std::stringstream() << taggedType.value).str());
        };
        ctmap::apply([&](auto const&... taggedTypes)
                     {
                         ((print(taggedTypes)), ...);
                     },
                     tagMap);
        out << (multiline ? '\n' : ' ') << '}';
        return std::ranges::copy(std::move(out).str(), context.out()).out;
    }
};
