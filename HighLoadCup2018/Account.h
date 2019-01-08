#pragma once

#include "SharedString.h"

#include <string>
#include <vector>
#include <set>

struct Like
{
    uint32_t id;
    int32_t ts;
};

struct Account
{
    using email_t = shared_string<0>;
    using first_name_t = shared_string<1>;
    using second_name_t = shared_string<2>;
    using country_t = shared_string<3>;
    using city_t = shared_string<4>;
    using interest_t = shared_string<5>;

    enum class Status : uint8_t
    {
        FREE = 0,
        BUSY = 1,
        COMPLICATED = 2
    };

    uint32_t id;
    std::string email;
    std::string email_domain;
    first_name_t first_name;
    second_name_t second_name;
    std::string phone;
    std::string phone_code;
    bool is_male;
    int32_t birth;
    uint16_t birth_year;
    country_t country;
    city_t city;
    int32_t joined;
    Status status;
    std::vector<interest_t> interest;
    int32_t premium_start = 0;
    int32_t premium_finish = 0;
    std::vector<uint32_t> like;

    const Account &account() const
    {
        return *this;
    }

    bool operator<(const Account &a) const
    {
        return id < a.id;
    }

    bool operator==(const Account &a) const
    {
        return id == a.id;
    }

    bool operator!=(const Account &a) const
    {
        return id != a.id;
    }
};
