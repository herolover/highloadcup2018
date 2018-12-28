#pragma once

#include <string>
#include <vector>
#include <set>

struct Like
{
    uint32_t id;
    uint32_t ts;
};

struct Account
{
    enum class Status
    {
        FREE = 0,
        BUSY = 1,
        COMPLICATED = 2
    };

    uint32_t id;
    std::string email;
    std::string email_domain;
    int8_t first_name_id = -1;
    int16_t second_name_id = -1;
    std::string phone;
    bool is_male;
    uint32_t birth;
    int8_t country_id = -1;
    int16_t city_id = -1;
    uint32_t joined;
    Status status;
    std::set<uint8_t> interest_id;
    uint32_t premium_start;
    uint32_t premium_finish;
    std::vector<Like> like;

    bool operator<(const Account &a) const
    {
        return id < a.id;
    }
};
