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

inline bool convert_sex(const std::string_view &value)
{
    return value[0] == 'm';
}

inline std::string_view convert_sex(bool is_male)
{
    return is_male ? "m"sv : "f"sv;
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

inline const char *convert_account_status(Account::Status status)
{
    switch (status)
    {
    case Account::Status::FREE:
        return u8"свободны";
    case Account::Status::BUSY:
        return u8"заняты";
    case Account::Status::COMPLICATED:
        return u8"всё сложно";
    default:
        return "";
    }
}
