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
    using RecommendIndex = mi::multi_index_container<AccountReference,
        mi::indexed_by<
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

    bool is_loaded = false;

    std::vector<Account::first_name_t> male_first_name;
    std::vector<Account::first_name_t> female_first_name;
    std::vector<std::unique_ptr<std::string>> interest_list;
    std::map<std::string_view, std::vector<AccountReference>> interest;
    std::map<std::string_view, RecommendIndex> interest_recommendations;

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
        if (!new_account.phone.empty() && account.get<phone_tag>().find(new_account.phone) != account.get<phone_tag>().end())
        {
            return false;
        }

        auto it = account.get<id_tag>().insert(std::move(new_account));
        if (it.second)
        {
            AccountReference account_reference(it.first);

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
                auto &interest_account_list = interest[account_interest];
                interest_account_list.insert(std::upper_bound(interest_account_list.begin(), interest_account_list.end(), account_reference), account_reference);
                interest_recommendations[account_interest].insert(account_reference);
            }

            for (auto &like : account_reference.account().like_list)
            {
                auto &liked_by_account_list = liked_by[like.id];
                liked_by_account_list.insert(std::upper_bound(liked_by_account_list.begin(), liked_by_account_list.end(), account_reference), account_reference);
            }

            if (is_loaded)
            {
                compute_interest_mask(it.first);
            }
        }

        return it.second;
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

    void compute_interest_mask()
    {
        is_loaded = true;
        auto &index = account.get<id_tag>();
        for (auto account_it = index.begin(); account_it != index.end(); ++account_it)
        {
            compute_interest_mask(account_it);
        }
    }

    template<class It>
    void compute_interest_mask(It account_it)
    {
        auto &index = account.get<id_tag>();
        Account::interest_mask_t interest_mask;
        for (auto &account_interest : account_it->interest_list)
        {
            interest_mask |= get_interest_mask(account_interest);
        }
        index.modify(account_it, [&interest_mask](Account &a)
        {
            a.interest_mask = interest_mask;
        });
    }
};
