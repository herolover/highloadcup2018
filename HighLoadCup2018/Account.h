#pragma once

#include "SharedString.h"

#include <string>
#include <vector>
#include <set>
#include <utility>
#include <bitset>

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
    using interest_mask_t = std::bitset<128>;

    enum class Status : uint8_t
    {
        FREE = 0,
        COMPLICATED = 1,
        BUSY = 2,
    };

    enum class PremiumStatus : uint8_t
    {
        NO = 0,
        EXPIRED = 1,
        ACTIVE = 2
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
    uint16_t joined_year;
    Status status;
    std::vector<interest_t> interest;
    interest_mask_t interest_mask;
    int32_t premium_start = 0;
    int32_t premium_finish = 0;
    PremiumStatus premium_status = PremiumStatus::NO;
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

    //std::size_t common_interest_size(const Account &a) const
    //{
    //    auto first1 = interest.begin();
    //    auto last1 = interest.end();
    //    auto first2 = a.interest.begin();
    //    auto last2 = a.interest.end();

    //    std::size_t counter = 0;
    //    while (first1 != last1 && first2 != last2)
    //    {
    //        if (string_view_compare()(*first1, *first2))
    //        {
    //            ++first1;
    //        }
    //        else if (string_view_compare()(*first2, *first1))
    //        {
    //            ++first2;
    //        }
    //        else
    //        {
    //            ++counter;
    //            ++first1;
    //            ++first2;
    //        }
    //    }

    //    return counter;
    //}

    int8_t status_compatibility(const Account &a) const
    {
        if (status == Status::FREE && a.status == Status::FREE)
        {
            return 4;
        }
        else if (status == Status::FREE && a.status == Status::COMPLICATED
                 || status == Status::COMPLICATED && a.status == Status::FREE)
        {
            return 3;
        }
        else if (status == Status::FREE && a.status == Status::BUSY
                 || status == Status::BUSY && a.status == Status::FREE)
        {
            return 2;
        }
        else if (status == Status::COMPLICATED && a.status == Status::BUSY
                 || status == Status::BUSY && a.status == Status::COMPLICATED)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    int32_t age_difference(const Account &a) const
    {
        return std::abs(birth - a.birth);
    }

    //bool is_more_compatible(const Account &a, const Account &b) const
    //{
    //    bool is_a_premium_now = a.premium_status == PremiumStatus::ACTIVE;
    //    bool is_b_premium_now = b.premium_status == PremiumStatus::ACTIVE;
    //    auto a_status_compatibility = status_compatibility(a);
    //    auto b_status_compatibility = status_compatibility(b);
    //    auto a_common_interests = common_interest_size(a);
    //    auto b_common_interests = common_interest_size(b);
    //    auto a_age_difference = age_difference(a);
    //    auto b_age_difference = age_difference(b);

    //    return std::tie(is_a_premium_now, a_status_compatibility, a_common_interests, b_age_difference, b.id) > std::tie(is_b_premium_now, b_status_compatibility, b_common_interests, a_age_difference, a.id);
    //}
};
