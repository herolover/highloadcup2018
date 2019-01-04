#pragma once

#include <string>
#include <string_view>
#include <set>
#include <algorithm>

template<class CharType>
std::vector<std::basic_string_view<CharType>> split(std::basic_string_view<CharType> value, const CharType delimiter = ',')
{
    std::vector<std::basic_string_view<CharType>> result;

    while (true)
    {
        auto delimiter_pos = value.find(delimiter);
        if (delimiter_pos != std::basic_string<CharType>::npos)
        {
            result.emplace_back(value.data(), delimiter_pos);
            value = {value.data() + delimiter_pos + 1, value.size() - delimiter_pos - 1};
        }
        else
        {
            result.emplace_back(value);
            break;
        }
    }

    std::sort(result.begin(), result.end());

    return result;
}


