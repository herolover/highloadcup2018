#pragma once

#include <string>
#include <string_view>
#include <set>
#include <algorithm>

template<class StringView>
std::vector<std::string_view> split(StringView value, const char delimiter = ',')
{
    std::vector<std::string_view> result;

    while (true)
    {
        auto delimiter_pos = value.find(delimiter);
        if (delimiter_pos != std::size_t(-1))
        {
            result.emplace_back(value.data(), delimiter_pos);
            value = {value.data() + delimiter_pos + 1, value.size() - delimiter_pos - 1};
        }
        else
        {
            result.emplace_back(value.data(), value.size());
            break;
        }
    }

    return result;
}


