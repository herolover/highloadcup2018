#pragma once

#include <string>
#include <string_view>
#include <set>

template<class CharType>
std::set<std::basic_string_view<CharType>> split(std::basic_string_view<CharType> value, const CharType delimiter = ',')
{
    std::set<std::basic_string_view<CharType>> result;

    while (true)
    {
        auto delimiter_pos = value.find(delimiter);
        if (delimiter_pos != std::basic_string<CharType>::npos)
        {
            result.emplace(value.data(), delimiter_pos);
            value = {value.data() + delimiter_pos + 1};
        }
        else
        {
            result.emplace(value);
            break;
        }
    }

    return result;
}


