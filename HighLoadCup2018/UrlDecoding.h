#pragma once

#include <string_view>
#include <charconv>

inline uint8_t hex_to_bin(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    else
    {
        return c - 'A' + 10;
    }
}

inline std::string decode_url(const std::string_view &value)
{
    std::string result;
    for (std::size_t i = 0; i < value.size(); ++i)
    {
        const char &c = value[i];
        if (c == '%')
        {
            char decoded_c = hex_to_bin(value[i + 1]) << 4 | hex_to_bin(value[i + 2]);
            result.push_back(decoded_c);
            i += 2;
        }
        else
        {
            result.push_back(c);
        }
    }

    return result;
}
