#pragma once
#include "char_tag.h"

#include <concepts>
#include <utility>


namespace ctmap
{
/**
* Wrapper supplying some type with a compile time tag.
*/
template<char_tag _Tag, typename _ValueType>
struct tagged_value
{
    constexpr static auto tag = _Tag;
    using value_type = _ValueType;

    template<typename ..._Args>
        requires std::constructible_from<value_type, _Args...>
    constexpr explicit tagged_value(_Args&&... args)
        : value(std::forward<_Args>(args)...)
    {}

    template<typename _OtherValueType>
        requires std::constructible_from<value_type, _OtherValueType>
    constexpr explicit tagged_value(tagged_value<_Tag, _OtherValueType>&& taggedValue)
        : value(std::forward<_OtherValueType&&>(taggedValue.value))
    {}

    template<typename _OtherValueType>
        requires std::constructible_from<value_type, _OtherValueType>
    constexpr explicit tagged_value(tagged_value<_Tag, _OtherValueType> const& taggedValue)
        : value(taggedValue.value)
    {}

    value_type value;
};

template<typename>
struct is_tagged_value : std::false_type
{};

template<typename _Type>
    requires requires(_Type) { _Type::tag; typename _Type::value_type; }
struct is_tagged_value<_Type> : std::is_same<_Type, tagged_value<_Type::tag, typename _Type::value_type>>
{};

template<typename _Type>
constexpr bool is_tagged_value_v = is_tagged_value<_Type>::value;

template<typename _Type>
concept TaggedValue = is_tagged_value_v<_Type>;

template<char_tag _Tag, typename _ValueType>
constexpr auto make_tagged(_ValueType&& value)
{
    return tagged_value<_Tag, std::decay_t<_ValueType>>(std::forward<_ValueType>(value));
}

template<char_tag _Tag, typename _ValueType>
constexpr auto make_tagged(std::reference_wrapper<_ValueType>&& value)
{
    return tagged_value<_Tag, _ValueType&>(value.get());
}

template<char_tag _Tag, typename _ValueType, typename... _ValueTypes>
constexpr auto make_tagged(_ValueTypes&&... values)
{
    return tagged_value<_Tag, _ValueType>(std::forward<_ValueTypes>(values)...);
}

template<char_tag _Tag, typename _ValueType>
constexpr auto tie_to_tag(_ValueType& value)
{
    return tagged_value<_Tag, _ValueType&>(value);
}

template<char_tag _Tag, typename _ValueType>
constexpr auto forward_as_tagged(_ValueType&& value)
{
    return tagged_value<_Tag, _ValueType&&>(std::forward<_ValueType>(value));
}

template<char_tag _Tag, typename _LhsValueType, typename _RhsValueType>
constexpr auto operator==(tagged_value<_Tag, _LhsValueType> const& lhs,
                          tagged_value<_Tag, _RhsValueType> const& rhs)
{
    return lhs.value == rhs.value;
}

template<char_tag _Tag, typename _LhsValueType, typename _RhsValueType>
constexpr auto operator<=>(tagged_value<_Tag, _LhsValueType> const& lhs,
                           tagged_value<_Tag, _RhsValueType> const& rhs)
{
    return lhs.value <=> rhs.value;
}
}
