#pragma once

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

namespace mi = boost::multi_index;

struct DB
{
    struct id_tag {};
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
    mi::multi_index_container<Account,
        mi::indexed_by<
            mi::ordered_unique<
                mi::tag<id_tag>,
                mi::member<Account, uint32_t, &Account::id>
            >,
            mi::ordered_unique<
                mi::tag<email_tag>,
                mi::member<Account, std::string, &Account::email>,
                common_less
            >,
            mi::ordered_non_unique<
                mi::tag<email_domain_tag>,
                mi::member<Account, std::string, &Account::email_domain>,
                common_less
            >,
            mi::ordered_non_unique<
                mi::tag<status_tag>,
                mi::member<Account, Account::Status, &Account::status>
            >,
            mi::ordered_non_unique<
                mi::tag<first_name_tag>,
                mi::member<Account, Account::first_name_t, &Account::first_name>,
                common_less
            >,
            mi::ordered_non_unique<
                mi::tag<second_name_tag>,
                mi::member<Account, Account::second_name_t, &Account::second_name>,
                common_less
            >,
            mi::ordered_non_unique<
                mi::tag<phone_tag>,
                mi::member<Account, std::string, &Account::phone>,
                common_less
            >,
            mi::ordered_non_unique<
                mi::tag<phone_code_tag>,
                mi::member<Account, std::string, &Account::phone_code>,
                common_less
            >,
            mi::ordered_non_unique<
                mi::tag<sex_tag>,
                mi::member<Account, Account::Sex, &Account::sex>
            >,
            mi::ordered_non_unique<
                mi::tag<birth_year_tag>,
                mi::member<Account, uint16_t, &Account::birth_year>
            >,
            mi::ordered_non_unique<
                mi::tag<country_tag>,
                mi::member<Account, Account::country_t, &Account::country>,
                common_less
            >,
            mi::ordered_non_unique<
                mi::tag<city_tag>,
                mi::member<Account, Account::city_t, &Account::city>,
                common_less
            >,
            mi::ordered_non_unique<
                mi::tag<joined_year_tag>,
                mi::member<Account, uint16_t, &Account::joined_year>
            >,
            mi::ordered_non_unique<
                mi::tag<premium_tag>,
                mi::member<Account, Account::PremiumStatus, &Account::premium_status>
            >
        >
    > account;
    // *INDENT-ON*

    using AccountIt = decltype(account)::index<id_tag>::type::iterator;

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

        Account::PremiumStatus premium_status() const
        {
            return _it->premium_status;
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
            // replace to random access
            mi::ordered_unique<
                mi::tag<id_tag>,
                mi::const_mem_fun<AccountReference, uint32_t, &AccountReference::id>
            >,
            mi::ordered_non_unique<
                mi::tag<recommend_tag>,
                mi::composite_key<AccountReference,
                    mi::const_mem_fun<AccountReference, Account::Sex, &AccountReference::sex>,
                    mi::const_mem_fun<AccountReference, Account::PremiumStatus, &AccountReference::premium_status>,
                    mi::const_mem_fun<AccountReference, Account::Status, &AccountReference::status>
                >
            >
        >
    >;
    // *INDENT-ON*

    std::mutex m;
    std::vector<Account::first_name_t> male_first_name;
    std::vector<Account::first_name_t> female_first_name;
    std::vector<std::unique_ptr<std::string>> interest_list;
    std::map<std::string_view, InterestIndex> interest_account_list;
    std::map<uint32_t, std::vector<AccountReference>> liked_by;

    std::string_view add_interest(std::string_view interest)
    {
        auto it = std::lower_bound(interest_list.begin(), interest_list.end(), interest, [](auto &&a, auto &&b)
        {
            return *a < b;
        });
        if (it == interest_list.end() || **it != interest)
        {
            it = interest_list.insert(it, std::make_unique<std::string>(interest.data(), interest.size()));
        }

        return std::string_view((*it)->data(), (*it)->size());
    }

    bool add_account(Account &&new_account)
    {
        std::lock_guard lock(m);
        if (!new_account.phone.empty() && account.get<phone_tag>().find(new_account.phone) != account.get<phone_tag>().end())
        {
            return false;
        }

        auto result = account.insert(std::move(new_account));
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
                interest_account_list[account_interest].insert(account_reference);
            }

            for (auto &like : account_reference.account().like_list)
            {
                auto &liked_by_account_list = liked_by[like.id];
                liked_by_account_list.insert(std::upper_bound(liked_by_account_list.begin(), liked_by_account_list.end(), account_reference), account_reference);
            }
        }

        return result.second;
    }

    bool update_account(Account &&update_account)
    {
        std::lock_guard lock(m);
        if (!update_account.email.empty() && account.get<email_tag>().find(update_account.email) != account.get<email_tag>().end())
        {
            return false;
        }
        if (!update_account.phone.empty() && account.get<phone_tag>().find(update_account.phone) != account.get<phone_tag>().end())
        {
            return false;
        }

        auto &index = account.get<id_tag>();
        auto it = index.find(update_account.id);
        if (it != index.end())
        {
            auto result = index.modify(it, [this, update_account = std::move(update_account), &it](Account &a) mutable
            {
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
                if (update_account.status != Account::Status::INVALID)
                {
                    a.status = update_account.status;
                }
                if (!update_account.interest_list.empty())
                {
                    AccountReference account_reference(it);
                    for (auto &account_interest : a.interest_list)
                    {
                        auto &account_list = interest_account_list[account_interest].get<id_tag>();
                        account_list.erase(account_list.find(account_reference.id()));
                    }

                    a.interest_list = std::move(update_account.interest_list);
                    a.interest_mask = update_account.interest_mask;

                    for (auto &account_interest : a.interest_list)
                    {
                        auto &account_list = interest_account_list[account_interest];
                        account_list.insert(account_reference);
                    }
                }
                if (update_account.premium_status != Account::PremiumStatus::INVALID)
                {
                    a.premium_status = update_account.premium_status;
                    a.premium_start = update_account.premium_start;
                    a.premium_finish = update_account.premium_finish;
                }
                if (!update_account.like_list.empty())
                {
                    AccountReference account_reference(it);
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
            });

            return result;
        }

        return false;
    }

    Account::interest_mask_t get_interest_mask(std::string_view interest)
    {
        Account::interest_mask_t mask;
        auto interest_it = std::lower_bound(interest_list.begin(), interest_list.end(), interest, [](auto &&a, auto &&b)
        {
            return *a < b;
        });
        if (**interest_it == interest)
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
        auto &index = account.get<id_tag>();
        for (auto account_it = index.begin(); account_it != index.end(); ++account_it)
        {
            index.modify(account_it, [this](Account &a)
            {
                a.interest_mask = get_interest_mask(a.interest_list);
            });
        }
    }
};
