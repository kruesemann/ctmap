# ctmap

ctmap is a header-only library for compile-time maps between unique string-literal tags and values of an assigned type.
The aim is to combine advantages of std::tuple and named members variables.

# Usage samples

## Basic usage

```cpp
#include "ctmap/ctmap.h"
#include <optional>
#include <string>

int main()
{
    ctmap::tag_map<
        ctmap::tagged_value<"tag1", std::string>,
        ctmap::tagged_value<"tag2", std::optional<std::string>>,
        ctmap::tagged_value<"tag3", int>
    > tagMap(
        "string",
        "optionalString",
        42
    );

    tagMap.get<"tag1">() = "newString";
    tagMap.get<"tag2">().reset();
    tagMap.get<2>().value += 3;
}
```

## Make tag map

```cpp
auto const tagMap = ctmap::make_tag_map(
    ctmap::make_tagged<"bool">(true),
    ctmap::make_tagged<"optionalUnsigned">(std::optional<unsigned int>(13u)),
    ctmap::make_tagged<"double">(6.66)
);
```

alternatively:

```cpp
auto const tagMap = ctmap::make_tag_map<
    ctmap::tagged_value<"bool", bool>,
    ctmap::tagged_value<"optionalUnsigned", std::optional<unsigned int>>,
    ctmap::tagged_value<"double", double>
>(true, 13u, 6.66);
```

or simpler:

```cpp
auto const tagMap = ctmap::make_tag_map<"bool", "optionalUnsigned", "double">(
    true,
    std::optional<unsigned int>(13u),
    6.66
);
```

## Concatenating tag maps

```cpp
auto const tagMap1 = ctmap::make_tag_map<"tag1", "tag2">(
    false,
    1.1f
);
auto const tagMap2 = ctmap::make_tag_map<"tag3">(
    "literal"
);
auto tagMap = ctmap::tag_map_cat(tagMap1, tagMap2);
static_assert(std::same_as<
                  decltype(tagMap),
                  ctmap::tag_map<
                      ctmap::tagged_value<"tag1", bool>,
                      ctmap::tagged_value<"tag2", float>,
                      ctmap::tagged_value<"tag3", char const(&)[8]>
                  >
              >);
```

## Getting a subset of a tag map

```cpp
auto const tagMap = ctmap::make_tag_map<"tag1", "tag2", "tag3">(
    false,
    1.1f,
    "literal"
);
auto smallerTagMap = ctmap::tag_map_cut<"tag1", "tag3">(tagMap);
static_assert(std::same_as<
                  decltype(smallerTagMap),
                  ctmap::tag_map<
                      ctmap::tagged_value<"tag1", bool>,
                      ctmap::tagged_value<"tag3", char const(&)[8]>
                  >
              >);
```

## Accessing multiple tagged values

```cpp
auto tagMap = ctmap::make_tag_map<"tag1", "tag2", "tag3">(
    std::string("value1.1"),
    42u,
    true
);
auto const& [value1, value2, value3] = tagMap.get<ctmap::all_tags>();
tagMap.get<"tag1", "tag3">() = std::make_tuple("value1.2", false);
auto const& [taggedValue1, taggedValue2, taggedValue3] = tagMap;
std::cout << "tag: " << taggedValue1.tag << ", value: " << taggedValue1.value << '\n';
```

## Applying functions to tag maps

```cpp
auto tagMap = ctmap::make_tag_map<"tag1", "tag2", "tag3">(
    std::string("value"),
    42u,
    true
);
tagMap.apply<"tag2", "tag3">([](auto& value2, auto& value3)
                             {
                                 value2 ^= 9u;
                                 value3 = !value3;
                             });
ctmap::apply([](auto const& ...taggedValues)
             {
                 ((std::cout << taggedValues.tag << ": " << taggedValues.value << '\n'), ...);
             },
             tagMap);
```

## std::format with tag maps

```cpp
#include "ctmap/ctmap.h"
#include "ctmap/formatter.h"
#include <iostream>
#include <format>

int main()
{
    auto const tagMap = ctmap::make_tag_map<"tag1", "tag2", "tag3">(
        std::string("value"),
        42u,
        true
    );
    std::cout << std::format("{}", tagMap) << '\n';
    std::cout << std::format("{0:m}", tagMap) << '\n';
}
```

output:

```
{ "tag1": "value", "tag2": "42", "tag3": "1" }
{
    "tag1": "value"
    "tag2": "42"
    "tag3": "1"
}
```
