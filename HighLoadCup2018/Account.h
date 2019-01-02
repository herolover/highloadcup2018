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
    using first_name_t = typename shared_string<0>;
    using second_name_t = typename shared_string<1>;
    using country_t = typename shared_string<2>;
    using city_t = typename shared_string<3>;
    using interest_t = typename shared_string<4>;

    enum class Status
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
    std::set<interest_t, string_view_compare> interest;
    int32_t premium_start = 0;
    int32_t premium_finish = 0;
    std::vector<Like> like;

    bool operator<(const Account &a) const
    {
        return id < a.id;
    }
};
