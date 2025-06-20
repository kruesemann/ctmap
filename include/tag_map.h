#pragma once
#include "char_tag.h"
#include "tagged_value.h"

#include <concepts>
#if __cpp_static_assert >= 202306L
#include <string>
#endif
#include <tuple>


namespace ctmap
{
template<TaggedValue... _TaggedValues>
class tag_map;

template<typename>
struct tag_map_from_tuple;

template<TaggedValue... _TaggedValue>
struct tag_map_from_tuple<std::tuple<_TaggedValue...>> : std::type_identity<tag_map<_TaggedValue...>>
{};

template<typename _TaggedValueTuple>
using tag_map_from_tuple_t = typename tag_map_from_tuple<_TaggedValueTuple>::type;

template<typename>
struct is_tag_map : std::false_type
{};

template<typename _Type>
    requires requires(_Type)
{
    typename _Type::tagged_tuple;
}
struct is_tag_map<_Type> : std::is_same<_Type, tag_map_from_tuple_t<typename _Type::tagged_tuple>>
{};

template<typename _Type>
constexpr bool is_tag_map_v = is_tag_map<_Type>::value;

template<typename _Type>
concept TagMap = is_tag_map_v<_Type>;

/**
* Compile time map between unique tags and assigned types.
* Iterable like a tuple, but every type has a name (in the form of a tag).
*/
template<TaggedValue... _TaggedValues>
class tag_map
{
    static_assert(is_unique_tag_list_v<_TaggedValues::tag...>, "tags are not unique");

    template<char_tag _Tag, size_t _Index>
    constexpr static size_t tag_index_impl()
    {
        return 0;
    }

    template<char_tag _Tag, size_t _Index, TaggedValue _TaggedValue, TaggedValue... _OtherTaggedValues>
    constexpr static size_t tag_index_impl()
    {
        if constexpr (_TaggedValue::tag == _Tag)
            return _Index;
        else
            return tag_index_impl<_Tag, _Index + 1, _OtherTaggedValues...>();
    }

    template<char_tag _Tag>
    constexpr static size_t tag_index_impl()
    {
        return tag_index_impl<_Tag, 0, _TaggedValues...>();
    }

public:

    using tagged_tuple = std::tuple<_TaggedValues...>;

    constexpr tag_map() noexcept = default;

    template<typename _Type>
        requires std::constructible_from<tagged_tuple, _Type>
    constexpr explicit tag_map(_Type&& value)
        : taggedValues(std::forward<_Type>(value))
    {}

    template<typename... _ValueTypes>
        requires (sizeof...(_ValueTypes) == sizeof...(_TaggedValues)) && (std::constructible_from<_TaggedValues, _ValueTypes> && ...)
    constexpr explicit tag_map(_ValueTypes&&... values)
        : taggedValues(std::forward_as_tuple(std::forward<_ValueTypes>(values)...))
    {}

    template<TaggedValue... _OtherTaggedValues>
        requires (sizeof...(_OtherTaggedValues) == sizeof...(_TaggedValues)) && (std::constructible_from<_TaggedValues, _OtherTaggedValues> && ...)
    constexpr explicit tag_map(tag_map<_OtherTaggedValues...>&& other)
        : taggedValues(std::move(other.taggedValues))
    {}

    template<TaggedValue... _OtherTaggedValues>
        requires (sizeof...(_OtherTaggedValues) == sizeof...(_TaggedValues)) && (std::constructible_from<_TaggedValues, _OtherTaggedValues> && ...)
    constexpr explicit tag_map(tag_map<_OtherTaggedValues...> const& other)
        : taggedValues(other.taggedValues)
    {}

    template<char_tag _Tag>
    constexpr static bool is_tag_valid()
    {
        return ((_Tag == _TaggedValues::tag) || ...);
    }

    template<char_tag _Tag>
    constexpr static size_t tag_index()
    {
        static_assert(is_tag_valid<_Tag>(),
#if __cpp_static_assert >= 202306L
                      std::string("'") + _Tag.value + std::string("' is not a valid tag for the tag map")
#else
                      "tag is not valid for the tag map"
#endif
        );
        return tag_index_impl<_Tag>();
    }

    template<size_t _Index>
    constexpr static size_t index_tag()
    {
        return std::tuple_element_t<_Index, tagged_tuple>::tag;
    }

    template<char_tag _Tag>
    using get_tagged_value_type_t = std::tuple_element_t<tag_index<_Tag>(), tagged_tuple>;
    template<char_tag _Tag>
    using get_tag_value_type_t = typename get_tagged_value_type_t<_Tag>::value_type;

    template<char_tag _Tag>
    constexpr auto& get()&
    {
        return std::get<tag_index<_Tag>()>(taggedValues).value;
    }

    template<char_tag _Tag>
        requires (!std::is_reference_v<get_tag_value_type_t<_Tag>>)
    constexpr auto const& get() const&
    {
        return std::get<tag_index<_Tag>()>(taggedValues).value;
    }

    template<char_tag _Tag>
        requires (std::is_reference_v<get_tag_value_type_t<_Tag>>)
    constexpr auto&& get() const&
    {
        return std::get<tag_index<_Tag>()>(taggedValues).value;
    }

    template<char_tag _Tag>
    constexpr auto&& get()&&
    {
        return std::get<tag_index<_Tag>()>(std::move(taggedValues)).value;
    }

    template<char_tag _Tag>
        requires (!std::is_reference_v<get_tag_value_type_t<_Tag>>)
    constexpr auto const&& get() const&&
    {
        return std::get<tag_index<_Tag>()>(std::move(taggedValues)).value;
    }

    template<char_tag _Tag>
        requires (std::is_reference_v<get_tag_value_type_t<_Tag>>)
    constexpr auto&& get() const&&
    {
        return std::get<tag_index<_Tag>()>(std::move(taggedValues)).value;
    }

    template<char_tag... _Tags>
        requires (sizeof...(_Tags) != 1)
    constexpr auto get()&
    {
        return std::tie(get<_Tags>()...);
    }

    template<char_tag... _Tags>
        requires (sizeof...(_Tags) != 1)
    constexpr auto get() const&
    {
        return std::tie(get<_Tags>()...);
    }

    template<char_tag... _Tags>
        requires (sizeof...(_Tags) != 1)
    constexpr auto get()&&
    {
        return std::forward_as_tuple(std::move(*this).template get<_Tags>()...);
    }

    template<char_tag... _Tags>
        requires (sizeof...(_Tags) != 1)
    constexpr auto get() const&&
    {
        return std::forward_as_tuple(std::move(*this).template get<_Tags>()...);
    }

    template<size_t _Index>
    constexpr auto& get()&
    {
        return std::get<_Index>(taggedValues);
    }

    template<size_t _Index>
    constexpr auto const& get() const&
    {
        return std::get<_Index>(taggedValues);
    }

    template<size_t _Index>
    constexpr auto&& get()&&
    {
        return std::get<_Index>(std::move(taggedValues));
    }

    template<size_t _Index>
    constexpr auto const&& get() const&&
    {
        return std::get<_Index>(std::move(taggedValues));
    }

    template<all_tags_t>
    constexpr auto get()&
    {
        return std::tie(get<_TaggedValues::tag>()...);
    }

    template<all_tags_t>
    constexpr auto get() const&
    {
        return std::tie(get<_TaggedValues::tag>()...);
    }

    template<all_tags_t>
    constexpr auto get()&&
    {
        return std::forward_as_tuple(std::move(*this).template get<_TaggedValues::tag>()...);
    }

    template<all_tags_t>
    constexpr auto get() const&&
    {
        return std::forward_as_tuple(std::move(*this).template get<_TaggedValues::tag>()...);
    }

    template<all_tags_t, typename _Function>
    constexpr auto apply(_Function&& function)&
    {
        return std::apply(std::forward<_Function>(function), get<all_tags>());
    }

    template<all_tags_t, typename _Function>
    constexpr auto apply(_Function&& function) const&
    {
        return std::apply(std::forward<_Function>(function), get<all_tags>());
    }

    template<all_tags_t, typename _Function>
    constexpr auto apply(_Function&& function)&&
    {
        return std::apply(std::forward<_Function>(function), std::move(*this).template get<all_tags>());
    }

    template<all_tags_t, typename _Function>
    constexpr auto apply(_Function&& function) const&&
    {
        return std::apply(std::forward<_Function>(function), std::move(*this).template get<all_tags>());
    }

    template<char_tag... _Tags, typename _Function>
    constexpr auto apply(_Function&& function)&
    {
        return std::apply(std::forward<_Function>(function), get<_Tags...>());
    }

    template<char_tag... _Tags, typename _Function>
    constexpr auto apply(_Function&& function) const&
    {
        return std::apply(std::forward<_Function>(function), get<_Tags...>());
    }

    template<char_tag... _Tags, typename _Function>
    constexpr auto apply(_Function&& function)&&
    {
        return std::apply(std::forward<_Function>(function), std::move(*this).template get<_Tags...>());
    }

    template<char_tag... _Tags, typename _Function>
    constexpr auto apply(_Function&& function) const&&
    {
        return std::apply(std::forward<_Function>(function), std::move(*this).template get<_Tags...>());
    }

private:

    template<TagMap _LhsTagMap, TagMap _RhsTagMap>
    friend constexpr auto operator==(_LhsTagMap const&,
                                     _RhsTagMap const&);

    template<TagMap _LhsTagMap, TagMap _RhsTagMap>
    friend constexpr auto operator<=>(_LhsTagMap const&,
                                      _RhsTagMap const&);

    template<typename _Function, TagMap _TagMap>
    friend constexpr auto apply(_Function&&,
                                _TagMap&);

    template<typename _Function, TagMap _TagMap>
    friend constexpr auto apply(_Function&&,
                                _TagMap const&);

    template<typename _Function, TagMap _TagMap>
    friend constexpr auto apply(_Function&&,
                                _TagMap&&);

    template<typename _Function, TagMap _TagMap>
    friend constexpr auto apply(_Function&&,
                                _TagMap const&&);

    template<char_tag... _Tags, typename _Function, TagMap _TagMap>
        requires (sizeof...(_Tags) > 0)
    friend constexpr auto apply(_Function&&,
                                _TagMap&);

    template<char_tag... _Tags, typename _Function, TagMap _TagMap>
        requires (sizeof...(_Tags) > 0)
    friend constexpr auto apply(_Function&&,
                                _TagMap const&);

    template<char_tag... _Tags, typename _Function, TagMap _TagMap>
        requires (sizeof...(_Tags) > 0)
    friend constexpr auto apply(_Function&&,
                                _TagMap&&);

    template<char_tag... _Tags, typename _Function, TagMap _TagMap>
        requires (sizeof...(_Tags) > 0)
    friend constexpr auto apply(_Function&&,
                                _TagMap const&&);

    template<TagMap... _TagMaps>
    friend constexpr auto tag_map_cat(_TagMaps&&...);

    template<TagMap... _TagMaps>
    friend constexpr auto tag_map_cat(_TagMaps const&...);

    tagged_tuple taggedValues;
};

template<TagMap _LhsTagMap, TagMap _RhsTagMap>
constexpr auto operator==(_LhsTagMap const& lhs,
                          _RhsTagMap const& rhs)
{
    return lhs.taggedValues == rhs.taggedValues;
}

template<TagMap _LhsTagMap, TagMap _RhsTagMap>
constexpr auto operator<=>(_LhsTagMap const& lhs,
                           _RhsTagMap const& rhs)
{
    return lhs.taggedValues <=> rhs.taggedValues;
}

template<char_tag ..._Tags, TagMap _TagMap>
constexpr auto& get(_TagMap& tagMap)
{
    return tagMap.template get<_Tags...>();
}

template<char_tag ..._Tags, TagMap _TagMap>
constexpr auto const& get(_TagMap const& tagMap)
{
    return tagMap.template get<_Tags...>();
}

template<char_tag ..._Tags, TagMap _TagMap>
constexpr auto&& get(_TagMap&& tagMap)
{
    return std::move(tagMap).template get<_Tags...>();
}

template<char_tag ..._Tags, TagMap _TagMap>
constexpr auto const&& get(_TagMap const&& tagMap)
{
    return std::move(tagMap).template get<_Tags...>();
}

template<all_tags_t , TagMap _TagMap>
constexpr auto get(_TagMap& tagMap)
{
    return tagMap.template get<all_tags>();
}

template<all_tags_t, TagMap _TagMap>
constexpr auto get(_TagMap const& tagMap)
{
    return tagMap.template get<all_tags>();
}

template<all_tags_t, TagMap _TagMap>
constexpr auto get(_TagMap&& tagMap)
{
    return std::move(tagMap).template get<all_tags>();
}

template<all_tags_t, TagMap _TagMap>
constexpr auto get(_TagMap const&& tagMap)
{
    return std::move(tagMap).template get<all_tags>();
}

template<size_t _Index, TagMap _TagMap>
constexpr auto& get(_TagMap& tagMap)
{
    return tagMap.template get<_Index>();
}

template<size_t _Index, TagMap _TagMap>
constexpr auto const& get(_TagMap const& tagMap)
{
    return tagMap.template get<_Index>();
}

template<size_t _Index, TagMap _TagMap>
constexpr auto&& get(_TagMap&& tagMap)
{
    return std::move(tagMap).template get<_Index>();
}

template<size_t _Index, TagMap _TagMap>
constexpr auto const&& get(_TagMap const&& tagMap)
{
    return std::move(tagMap).template get<_Index>();
}

template<typename _Function, TagMap _TagMap>
constexpr auto apply(_Function&& function,
                     _TagMap& tagMap)
{
    return std::apply(std::forward<_Function>(function), tagMap.taggedValues);
}

template<typename _Function, TagMap _TagMap>
constexpr auto apply(_Function&& function,
                     _TagMap const& tagMap)
{
    return std::apply(std::forward<_Function>(function), tagMap.taggedValues);
}

template<typename _Function, TagMap _TagMap>
constexpr auto apply(_Function&& function,
                     _TagMap&& tagMap)
{
    return std::apply(std::forward<_Function>(function), std::move(tagMap.taggedValues));
}

template<typename _Function, TagMap _TagMap>
constexpr auto apply(_Function&& function,
                     _TagMap const&& tagMap)
{
    return std::apply(std::forward<_Function>(function), std::move(tagMap.taggedValues));
}

template<char_tag... _Tags, typename _Function, TagMap _TagMap>
    requires (sizeof...(_Tags) > 0)
constexpr auto apply(_Function&& function,
                     _TagMap& tagMap)
{
    return std::apply(std::forward<_Function>(function),
                      std::tie(std::get<_TagMap::template tag_index<_Tags>()>(tagMap.taggedValues)...));
}

template<char_tag... _Tags, typename _Function, TagMap _TagMap>
    requires (sizeof...(_Tags) > 0)
constexpr auto apply(_Function&& function,
                     _TagMap const& tagMap)
{
    return std::apply(std::forward<_Function>(function),
                      std::tie(std::get<_TagMap::template tag_index<_Tags>()>(tagMap.taggedValues)...));
}

template<char_tag... _Tags, typename _Function, TagMap _TagMap>
    requires (sizeof...(_Tags) > 0)
constexpr auto apply(_Function&& function,
                     _TagMap&& tagMap)
{
    return std::apply(std::forward<_Function>(function),
                      std::forward_as_tuple(std::get<_TagMap::template tag_index<_Tags>()>(std::forward<decltype(tagMap.taggedValues)>(tagMap.taggedValues))...));
}

template<char_tag... _Tags, typename _Function, TagMap _TagMap>
    requires (sizeof...(_Tags) > 0)
constexpr auto apply(_Function&& function,
                     _TagMap const&& tagMap)
{
    return std::apply(std::forward<_Function>(function),
                      std::forward_as_tuple(std::get<_TagMap::template tag_index<_Tags>()>(std::forward<decltype(tagMap.taggedValues)>(tagMap.taggedValues))...));
}

template<TaggedValue... _TaggedValues>
constexpr auto make_tag_map(_TaggedValues&&... values)
{
    return tag_map<_TaggedValues...>(std::forward<_TaggedValues>(values)...);
}

template<TaggedValue... _TaggedValues>
constexpr auto make_tag_map(std::tuple<_TaggedValues...> const& taggedValueTuple)
{
    return tag_map<_TaggedValues...>(taggedValueTuple);
}

template<TaggedValue... _TaggedValues>
constexpr auto make_tag_map(std::tuple<_TaggedValues...>&& taggedValueTuple)
{
    return tag_map<_TaggedValues...>(std::move(taggedValueTuple));
}

template<TaggedValue... _TaggedValues, typename... _ValueTypes>
    requires (sizeof...(_TaggedValues) > 0) && (std::constructible_from<_TaggedValues, _ValueTypes> && ...)
constexpr auto make_tag_map(_ValueTypes&&... values)
{
    static_assert(sizeof...(_TaggedValues) == sizeof...(_ValueTypes), "sizes of tagged values and values mismatch");
    return tag_map<_TaggedValues...>(std::forward<_ValueTypes>(values)...);
}

template<char_tag... _Tags, typename... _ValueTypes>
    requires (sizeof...(_Tags) > 0)
constexpr auto make_tag_map(_ValueTypes&&... values)
{
    static_assert(sizeof...(_Tags) == sizeof...(_ValueTypes), "sizes of tags and values mismatch");
    return tag_map<decltype(make_tagged<_Tags>(std::forward<_ValueTypes>(values)))...>(make_tagged<_Tags>(std::forward<_ValueTypes>(values))...);
}

template<char_tag... _Tags, typename... _ValueTypes>
    requires (sizeof...(_Tags) > 0)
constexpr auto tie_tag_map(_ValueTypes&... values)
{
    static_assert(sizeof...(_Tags) == sizeof...(_ValueTypes), "sizes of tags and values mismatch");
    return tag_map<tagged_value<_Tags, _ValueTypes&>...>(values...);
}

template<char_tag... _Tags, typename... _ValueTypes>
    requires (sizeof...(_Tags) > 0)
constexpr auto forward_as_tag_map(_ValueTypes&&... values)
{
    static_assert(sizeof...(_Tags) == sizeof...(_ValueTypes), "sizes of tags and values mismatch");
    return tag_map<tagged_value<_Tags, _ValueTypes&&>...>(std::forward<_ValueTypes>(values)...);
}

template<TagMap... _TagMaps>
constexpr auto tag_map_cat(_TagMaps&&... tagMaps)
{
    return make_tag_map(std::tuple_cat(std::move(tagMaps.taggedValues)...));
}

template<TagMap... _TagMaps>
constexpr auto tag_map_cat(_TagMaps const&... tagMaps)
{
    return make_tag_map(std::tuple_cat(tagMaps.taggedValues...));
}

template<TagMap _TagMap, char_tag... _Tags>
struct cut_tag_map : std::type_identity<tag_map<typename _TagMap::template get_tagged_value_type_t<_Tags>...>>
{};

template<TagMap _TagMap, char_tag... _Tags>
using cut_tag_map_t = typename cut_tag_map<_TagMap, _Tags...>::type;

template<char_tag... _Tags, TagMap _TagMap>
constexpr auto tag_map_cut(_TagMap&& tagMap)
{
    return cut_tag_map_t<_TagMap, _Tags...>(std::move(tagMap).template get<_Tags...>());
}

template<char_tag... _Tags, TagMap _TagMap>
constexpr auto tag_map_cut(_TagMap const& tagMap)
{
    return cut_tag_map_t<_TagMap, _Tags...>(tagMap.template get<_Tags...>());
}
}

template<ctmap::TagMap _TagMap>
struct std::tuple_size<_TagMap> : std::tuple_size<typename _TagMap::tagged_tuple>
{};

template<size_t _Index, ctmap::TagMap _TagMap>
struct std::tuple_element<_Index, _TagMap> : std::tuple_element<_Index, typename _TagMap::tagged_tuple>
{};
