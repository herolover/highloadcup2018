#pragma once

#include "Account.h"

#include <ctime>
#include <string_view>

using namespace std::literals;

inline uint16_t get_year(std::time_t timestamp)
{
    std::tm *time = std::gmtime(&timestamp);
    return time->tm_year + 1900;
}

inline bool is_male(const std::string_view &value)
{
    return value[0] == 'm';
}

inline Account::Status convert_account_status(const std::string_view &value)
{
    if (value == u8"свободны"sv)
    {
        return Account::Status::FREE;
    }
    else if (value == u8"заняты"sv)
    {
        return Account::Status::BUSY;
    }
    else
    {
        return Account::Status::COMPLICATED;
    }
}
