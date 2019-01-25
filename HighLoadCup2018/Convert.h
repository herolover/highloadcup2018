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

inline Account::Sex convert_sex(const std::string_view &value)
{
    return value[0] == 'm' ? Account::Sex::MALE : (value[0] == 'f' ? Account::Sex::FEMALE : Account::Sex::INVALID);
}

inline const char *convert_sex(Account::Sex sex)
{
    return sex == Account::Sex::MALE ? "m" : "f";
}

inline Account::Status convert_account_status(const std::string_view &value)
{
    Account::Status result = Account::Status::INVALID;

    if (value == u8"свободны"sv)
    {
        result = Account::Status::FREE;
    }
    else if (value == u8"заняты"sv)
    {
        result = Account::Status::BUSY;
    }
    else if (value == u8"всё сложно"sv)
    {
        result = Account::Status::COMPLICATED;
    }

    return result;
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
