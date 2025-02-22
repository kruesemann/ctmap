#pragma once
#include <concepts>


namespace ctmap
{
/**
* Helper class for simplifying usage of string literals as template arguments.
* Without, some_template<"some_string_literal"> would not work.
*/
template<size_t _Size>
struct char_tag
{
    char value[_Size];

    constexpr char_tag(char const (&_value)[_Size]) noexcept
    {
        for (size_t index = 0; index < _Size; ++index)
            value[index] = _value[index];
    }

    constexpr operator char const*() const
    {
        return value;
    }
};

template<size_t _LhsSize, size_t _RhsSize>
constexpr bool operator==(char_tag<_LhsSize> const& lhs,
                          char_tag<_RhsSize> const& rhs)
{
    if constexpr (_LhsSize != _RhsSize)
        return false;
    for (size_t index = 0; index < _LhsSize; ++index)
        if (lhs.value[index] != rhs.value[index])
            return false;
    return true;
}

template<char_tag _Tag, char_tag... _Tags>
constexpr bool is_unique_tag_list()
{
    if constexpr (sizeof...(_Tags) > 0)
    {
        return ((_Tag != _Tags) && ...)
            && is_unique_tag_list<_Tags...>();
    }
    return true;
}

template<char_tag... _Tags>
constexpr bool is_unique_tag_list_v = is_unique_tag_list<_Tags...>();

struct all_tags_t
{};

constexpr all_tags_t all_tags;
}
