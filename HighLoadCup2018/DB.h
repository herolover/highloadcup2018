#pragma once

#include "SpinLock.h"
#include "Account.h"
#include "FieldMethod.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>

#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/include/vector.hpp>

#include <set>
#include <map>
#include <algorithm>
#include <optional>
#include <functional>
#include <unordered_map>
#include <tuple>
#include <mutex>
#include <array>

namespace mi = boost::multi_index;

struct NewLike
{
    uint32_t likee_id = 0;
    uint32_t liker_id = 0;
    int32_t like_ts = 0;
};

struct DB
{
    struct email_tag {};
    struct email_domain_tag {};
    struct first_name_tag {};
    struct second_name_tag {};
    struct phone_tag {};
    struct phone_code_tag {};
    struct sex_tag {};
    struct birth_year_tag {};
    struct country_tag {};
    struct city_tag {};
    struct joined_year_tag {};
    struct status_tag {};
    struct premium_tag {};
    struct interest_tag {};
    struct like_tag {};

    // *INDENT-OFF*
    using AccountIndex = mi::multi_index_container<Account,
        mi::indexed_by<
            mi::random_access<>,
            mi::ordered_unique<
                mi::tag<email_tag>,
                mi::composite_key<Account,
                    mi::member<Account, std::string, &Account::email>,
                    mi::member<Account, uint32_t, &Account::id>
                >,
                mi::composite_key_compare<
                    common_less,
                    std::less<uint32_t>
                >
            >,
            mi::ordered_non_unique<
                mi::tag<email_domain_tag>,
                mi::composite_key<Account,
                    mi::member<Account, std::string, &Account::email_domain>,
                    mi::member<Account, uint32_t, &Account::id>
                >,
                mi::composite_key_compare<
                    common_less,
                    std::less<uint32_t>
                >
            >,
            mi::ordered_non_unique<
                mi::tag<status_tag>,
                mi::composite_key<Account,
                    mi::member<Account, Account::Status, &Account::status>,
                    mi::member<Account, uint32_t, &Account::id>
                >
            >,
            mi::ordered_non_unique<
                mi::tag<first_name_tag>,
                mi::composite_key<Account,
                    mi::member<Account, Account::first_name_t, &Account::first_name>,
                    mi::member<Account, uint32_t, &Account::id>
                >,
                mi::composite_key_compare<
                    common_less,
                    std::less<uint32_t>
                >
            >,
            mi::ordered_non_unique<
                mi::tag<second_name_tag>,
                mi::composite_key<Account,
                    mi::member<Account, Account::second_name_t, &Account::second_name>,
                    mi::member<Account, uint32_t, &Account::id>
                >,
                mi::composite_key_compare<
                    common_less,
                    std::less<uint32_t>
                >
            >,
            mi::ordered_non_unique<
                mi::tag<phone_tag>,
                mi::composite_key<Account,
                    mi::member<Account, std::string, &Account::phone>,
                    mi::member<Account, uint32_t, &Account::id>
                >,
                mi::composite_key_compare<
                    common_less,
                    std::less<uint32_t>
                >
            >,
            mi::ordered_non_unique<
                mi::tag<phone_code_tag>,
                mi::composite_key<Account,
                    mi::member<Account, std::string, &Account::phone_code>,
                    mi::member<Account, uint32_t, &Account::id>
                >,
                mi::composite_key_compare<
                    common_less,
                    std::less<uint32_t>
                >
            >,
            mi::ordered_non_unique<
                mi::tag<sex_tag>,
                mi::composite_key<Account,
                    mi::member<Account, Account::Sex, &Account::sex>,
                    mi::member<Account, uint32_t, &Account::id>
                >
            >,
            mi::ordered_non_unique<
                mi::tag<birth_year_tag>,
                mi::composite_key<Account,
                    mi::member<Account, uint16_t, &Account::birth_year>,
                    mi::member<Account, uint32_t, &Account::id>
                >
            >,
            mi::ordered_non_unique<
                mi::tag<country_tag>,
                mi::composite_key<Account,
                    mi::member<Account, Account::country_t, &Account::country>,
                    mi::member<Account, uint32_t, &Account::id>
                >,
                mi::composite_key_compare<
                    common_less,
                    std::less<uint32_t>
                >
            >,
            mi::ordered_non_unique<
                mi::tag<city_tag>,
                mi::composite_key<Account,
                    mi::member<Account, Account::city_t, &Account::city>,
                    mi::member<Account, uint32_t, &Account::id>
                >,
                mi::composite_key_compare<
                    common_less,
                    std::less<uint32_t>
                >
            >,
            mi::ordered_non_unique<
                mi::tag<joined_year_tag>,
                mi::composite_key<Account,
                    mi::member<Account, uint16_t, &Account::joined_year>,
                    mi::member<Account, uint32_t, &Account::id>
                >
            >,
            mi::ordered_non_unique<
                mi::tag<premium_tag>,
                mi::composite_key<Account,
                    mi::member<Account, Account::PremiumStatus, &Account::premium_status>,
                    mi::member<Account, uint32_t, &Account::id>
                >
            >
        >
    >;
    // *INDENT-ON*

    using AccountIt = AccountIndex::iterator;

    class AccountReference
    {
    public:
        AccountReference(AccountIt it)
            : _it(it)
        {
        }

        uint32_t id() const
        {
            return _it->id;
        }

        std::string_view city() const
        {
            return *_it->city;
        }

        std::string_view country() const
        {
            return *_it->country;
        }

        Account::Sex sex() const
        {
            return _it->sex;
        }

        Account::Status status() const
        {
            return _it->status;
        }

        bool is_not_premium_now() const
        {
            return _it->premium_status != Account::PremiumStatus::ACTIVE;
        }

        const Account &account() const
        {
            return *_it;
        }

        auto &operator*() const
        {
            return *_it;
        }

        AccountReference(const AccountReference &) = default;
        AccountReference(AccountReference &&) = default;
        AccountReference &operator=(const AccountReference &) = default;
        AccountReference &operator=(AccountReference &&) = default;

        bool operator<(const AccountReference &another) const
        {
            return _it->id < another._it->id;
        }

        bool operator==(const AccountReference &another) const
        {
            return _it->id == another._it->id;
        }

        bool operator!=(const AccountReference &another) const
        {
            return _it->id != another._it->id;
        }

    private:
        AccountIt _it;
    };

    struct recommend_tag {};
    // *INDENT-OFF*
    using InterestIndex = mi::multi_index_container<AccountReference,
        mi::indexed_by<
            mi::random_access<>,
            mi::ordered_non_unique<
                mi::tag<recommend_tag>,
                mi::composite_key<AccountReference,
                    mi::const_mem_fun<AccountReference, Account::Sex, &AccountReference::sex>,
                    mi::const_mem_fun<AccountReference, bool, &AccountReference::is_not_premium_now>,
                    mi::const_mem_fun<AccountReference, Account::Status, &AccountReference::status>,
                    mi::const_mem_fun<AccountReference, uint32_t, &AccountReference::id>
                >
            >
        >
    >;
    // *INDENT-ON*

    struct Group
    {
        Account::city_t city;
        Account::country_t country;
        Account::Status status;
        Account::Sex sex;

        std::size_t count = 0;
        std::map<uint16_t, std::size_t> birth_year_count;
        std::map<uint16_t, std::size_t> joined_year_count;

        Group(const Account &a)
            : city(a.city)
            , country(a.country)
            , status(a.status)
            , sex(a.sex)
        {
        }
        Group(const Group &) = default;
        Group(Group &&) = default;

        Group &operator=(const Group &) = default;
        Group &operator=(Group &&) = default;

        void add_account(const Account &a)
        {
            ++count;
            ++birth_year_count[a.birth_year];
            ++joined_year_count[a.joined_year];
        }

        void remove_account(const Account &a)
        {
            --count;
            --birth_year_count[a.birth_year];
            --joined_year_count[a.joined_year];
        }

        bool operator<(const Group &g) const
        {
            return std::tie(city, country, status, sex) < std::tie(g.city, g.country, g.status, g.sex);
        }

        bool operator==(const Group &g) const
        {
            return std::tie(city, country, status, sex) == std::tie(g.city, g.country, g.status, g.sex);
        }
    };

    struct GroupIndex
    {
        std::vector<Group> group_list;

        std::pair<bool, std::vector<Group>::iterator> find_group(const Group &group)
        {
            auto it = std::lower_bound(group_list.begin(), group_list.end(), group);
            return std::make_pair(it != group_list.end() && *it == group, it);
        }

        void add_account(const Account &account)
        {
            Group group(account);
            auto res = find_group(group);
            if (res.first)
            {
                res.second->add_account(account);
            }
            else
            {
                group.add_account(account);
                group_list.insert(res.second, std::move(group));
            }
        }

        void remove_account(const Account &account)
        {
            Group group(account);
            auto res = find_group(group);
            if (res.first)
            {
                res.second->remove_account(account);
            }
        }
    };

    spin_lock m;
    AccountIndex account;
    std::vector<Account::first_name_t> male_first_name;
    std::vector<Account::first_name_t> female_first_name;
    std::vector<std::unique_ptr<std::string>> interest_list;
    std::map<std::string_view, InterestIndex> interest_account_list;
    std::map<uint32_t, std::vector<AccountReference>> liked_by;
    GroupIndex group_index;

    int32_t current_time = 0;
    std::size_t initial_account_size = 0;

    std::string_view add_interest(std::string_view interest)
    {
        auto it = std::lower_bound(interest_list.begin(), interest_list.end(), interest, [](auto &&a, auto &&b)
        {
            return *a < b;
        });
        if (it == interest_list.end() || **it != interest)
        {
            it = interest_list.insert(it, std::make_unique<std::string>(interest.data(), interest.size()));

            interest_account_list[std::string_view((*it)->data(), (*it)->size())].reserve(50'000);
        }

        return std::string_view((*it)->data(), (*it)->size());
    }

    bool is_initial_account(uint32_t account_id)
    {
        return account_id <= initial_account_size;
    }

    auto find_account(uint32_t account_id)
    {
        if (is_initial_account(account_id))
        {
            return account.begin() + (account_id - 1);
        }

        return std::lower_bound(account.begin(), account.end(), account_id, [](const Account &a, uint32_t account_id)
        {
            return a.id < account_id;
        });
    }

    std::pair<bool, AccountIt> has_account(uint32_t account_id)
    {
        auto it = find_account(account_id);
        return std::make_pair(it != account.end() && it->id == account_id, it);
    }

    template<std::size_t N>
    bool add_like_list(const std::array<NewLike, N> &like_list, std::size_t like_list_size)
    {
        std::lock_guard lock(m);
        for (std::size_t i = 0; i < like_list_size; ++i)
        {
            auto &like = like_list[i];
            if (!has_account(like.likee_id).first || !has_account(like.liker_id).first)
            {
                return false;
            }
        }

        for (std::size_t i = 0; i < like_list_size; ++i)
        {
            auto &like = like_list[i];

            auto liker_it = find_account(like.liker_id);
            auto likee_id = like.likee_id;
            auto like_ts = like.like_ts;
            account.modify(liker_it, [likee_id, like_ts](Account &a)
            {
                a.add_like(likee_id, like_ts);
            });
            AccountReference account_reference(liker_it);
            auto &account_list = liked_by[likee_id];
            account_list.insert(std::upper_bound(account_list.begin(), account_list.end(), account_reference), account_reference);
        }

        return true;
    }

    bool add_account(Account &&new_account, bool check_likes = true)
    {
        std::lock_guard lock(m);
        if (!new_account.phone.empty() && account.get<phone_tag>().find(new_account.phone) != account.get<phone_tag>().end())
        {
            return false;
        }
        if (check_likes)
        {
            for (auto &like : new_account.like_list)
            {
                if (!has_account(like.id).first)
                {
                    return false;
                }
            }
        }

        auto result = account.insert(std::upper_bound(account.begin(), account.end(), new_account.id, [](uint32_t account_id, const Account &a)
        {
            return account_id < a.id;
        }), std::move(new_account));
        if (result.second)
        {
            AccountReference account_reference(result.first);

            if (account_reference.account().first_name)
            {
                auto &first_name_list = account_reference.account().sex == Account::Sex::MALE ? male_first_name : female_first_name;
                auto it = std::lower_bound(first_name_list.begin(), first_name_list.end(), account_reference.account().first_name);
                if (it == first_name_list.end() || **it != *account_reference.account().first_name)
                {
                    first_name_list.insert(it, account_reference.account().first_name);
                }
            }

            for (auto &account_interest : account_reference.account().interest_list)
            {
                auto &account_list = interest_account_list[account_interest];
                account_list.insert(std::upper_bound(account_list.begin(), account_list.end(), account_reference), account_reference);
            }

            for (auto &like : account_reference.account().like_list)
            {
                auto &liked_by_account_list = liked_by[like.id];
                liked_by_account_list.insert(std::upper_bound(liked_by_account_list.begin(), liked_by_account_list.end(), account_reference), account_reference);
            }

            group_index.add_account(account_reference.account());
        }

        return result.second;
    }

    enum class UpdateAccountResult
    {
        SUCCESS,
        INVALID_ACCOUNT_DATA,
        ACCOUNT_NOT_FOUND
    };

    UpdateAccountResult update_account(Account &&update_account)
    {
        std::lock_guard lock(m);
        auto account_result = has_account(update_account.id);
        if (!account_result.first)
        {
            return UpdateAccountResult::ACCOUNT_NOT_FOUND;
        }
        if (!update_account.email.empty() && account.get<email_tag>().find(update_account.email) != account.get<email_tag>().end())
        {
            return UpdateAccountResult::INVALID_ACCOUNT_DATA;
        }
        if (!update_account.phone.empty() && account.get<phone_tag>().find(update_account.phone) != account.get<phone_tag>().end())
        {
            return UpdateAccountResult::INVALID_ACCOUNT_DATA;
        }
        for (auto &like : update_account.like_list)
        {
            if (!has_account(like.id).first)
            {
                return UpdateAccountResult::INVALID_ACCOUNT_DATA;
            }
        }

        auto account_it = account_result.second;
        account.modify(account_it, [this, update_account = std::move(update_account), account_it](Account &a) mutable
        {
            group_index.remove_account(a);

            if (!update_account.email.empty())
            {
                a.email = std::move(update_account.email);
                a.email_domain = std::move(update_account.email_domain);
            }
            if (update_account.first_name)
            {
                a.first_name = update_account.first_name;
            }
            if (update_account.second_name)
            {
                a.second_name = update_account.second_name;
            }
            if (!update_account.phone.empty())
            {
                a.phone = std::move(update_account.phone);
                a.phone_code = std::move(update_account.phone_code);
            }
            if (update_account.sex != Account::Sex::INVALID)
            {
                a.sex = update_account.sex;
            }
            if (update_account.birth != 0)
            {
                a.birth = update_account.birth;
                a.birth_year = update_account.birth_year;
            }
            if (update_account.country)
            {
                a.country = update_account.country;
            }
            if (update_account.city)
            {
                a.city = update_account.city;
            }
            if (update_account.joined != 0)
            {
                a.joined = update_account.joined;
                a.joined_year = update_account.joined_year;
            }
            bool update_interests = false;
            if (update_account.status != Account::Status::INVALID)
            {
                a.status = update_account.status;
                update_interests = true;
            }
            if (update_account.premium_status != Account::PremiumStatus::NO)
            {
                a.premium_status = update_account.premium_status;
                a.premium_start = update_account.premium_start;
                a.premium_finish = update_account.premium_finish;
                update_interests = true;
            }
            if (!update_account.interest_list.empty())
            {
                AccountReference account_reference(account_it);
                for (auto &account_interest : a.interest_list)
                {
                    auto &account_list = interest_account_list[account_interest];
                    account_list.erase(std::lower_bound(account_list.begin(), account_list.end(), account_reference));
                }

                a.interest_list = std::move(update_account.interest_list);
                a.interest_mask = update_account.interest_mask;

                for (auto &account_interest : a.interest_list)
                {
                    auto &account_list = interest_account_list[account_interest];
                    account_list.insert(std::upper_bound(account_list.begin(), account_list.end(), account_reference), account_reference);
                }
            }
            else if (update_interests)
            {
                AccountReference account_reference(account_it);
                for (auto &account_interest : a.interest_list)
                {
                    auto &account_list = interest_account_list[account_interest];
                    account_list.erase(std::lower_bound(account_list.begin(), account_list.end(), account_reference));
                }

                for (auto &account_interest : a.interest_list)
                {
                    auto &account_list = interest_account_list[account_interest];
                    account_list.insert(std::upper_bound(account_list.begin(), account_list.end(), account_reference), account_reference);
                }
            }
            if (!update_account.like_list.empty())
            {
                AccountReference account_reference(account_it);
                for (auto &like : a.like_list)
                {
                    auto &account_list = liked_by[like.id];
                    account_list.erase(std::lower_bound(account_list.begin(), account_list.end(), account_reference));
                }

                a.like_list = std::move(update_account.like_list);

                for (auto &like : a.like_list)
                {
                    auto &account_list = liked_by[like.id];
                    account_list.insert(std::upper_bound(account_list.begin(), account_list.end(), account_reference), account_reference);
                }
            }

            group_index.add_account(a);
        });

        return UpdateAccountResult::SUCCESS;
    }

    Account::interest_mask_t get_interest_mask(std::string_view interest)
    {
        Account::interest_mask_t mask;
        auto interest_it = std::lower_bound(interest_list.begin(), interest_list.end(), interest, [](auto &&a, auto &&b)
        {
            return *a < b;
        });
        if (interest_it != interest_list.end() && **interest_it == interest)
        {
            mask[std::distance(interest_list.begin(), interest_it)] = true;
        }
        else
        {
            mask[mask.size() - 1] = true;
        }

        return mask;
    }

    Account::interest_mask_t get_interest_mask(const std::vector<std::string_view> &interest_list)
    {
        Account::interest_mask_t mask;
        for (auto &interest : interest_list)
        {
            mask |= get_interest_mask(interest);
        }

        return mask;
    }

    void compute_interest_mask()
    {
        for (auto account_it = account.begin(); account_it != account.end(); ++account_it)
        {
            account.modify(account_it, [this](Account &a)
            {
                a.interest_mask = get_interest_mask(a.interest_list);
            });
        }
    }
};
