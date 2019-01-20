#pragma once

#include "SharedString.h"

#include <string>
#include <vector>
#include <set>
#include <utility>
#include <bitset>
#include <algorithm>

struct Like
{
    uint32_t id;
    int32_t ts;

    Like(uint32_t id, int32_t ts)
        : id(id)
        , ts(ts)
    {
    }

    bool operator<(const Like &like) const
    {
        return id < like.id;
    }

    struct is_less
    {
        using is_transparent = void;

        bool operator()(const Like &like, uint32_t id)
        {
            return like.id < id;
        }

        bool operator()(uint32_t id, const Like &like)
        {
            return id < like.id;
        }
    };
};

struct Account
{
    using first_name_t = shared_string<0>;
    using second_name_t = shared_string<1>;
    using country_t = shared_string<2>;
    using city_t = shared_string<3>;
    using interest_mask_t = std::bitset<128>;

    enum class Sex: uint8_t
    {
        INVALID = 0,
        MALE = 1,
        FEMALE = 2
    };

    enum class Status : uint8_t
    {
        INVALID = 0,
        FREE = 1,
        COMPLICATED = 2,
        BUSY = 3,
    };

    enum class PremiumStatus : uint8_t
    {
        INVALID = 0,
        ACTIVE = 1,
        NO = 2,
        EXPIRED = 3,
    };

    uint32_t id = 0;
    std::string email;
    std::string email_domain;
    first_name_t first_name;
    second_name_t second_name;
    std::string phone;
    std::string phone_code;
    Sex sex = Sex::INVALID;
    int32_t birth = 0;
    uint16_t birth_year = 0;
    country_t country;
    city_t city;
    int32_t joined = 0;
    uint16_t joined_year = 0;
    Status status = Status::INVALID;
    std::vector<std::string_view> interest_list;
    interest_mask_t interest_mask;
    int32_t premium_start = 0;
    int32_t premium_finish = 0;
    PremiumStatus premium_status = PremiumStatus::NO;
    std::vector<Like> like_list;

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

    void add_like(uint32_t account_id, int32_t like_ts)
    {
        auto it = std::upper_bound(like_list.begin(), like_list.end(), account_id, Like::is_less());
        like_list.emplace(it, account_id, like_ts);
    }

    void add_interest(std::string_view interest)
    {
        auto it = std::upper_bound(interest_list.begin(), interest_list.end(), interest);
        interest_list.insert(it, interest);
    }
};
