#pragma once

#include "Account.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include <set>
#include <map>
#include <algorithm>

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
    struct birth_tag {};
    struct birth_year_tag {};
    struct country_tag {};
    struct city_tag {};
    struct joined_tag {};
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
                mi::member<Account, Account::email_t, &Account::email>,
                string_view_compare
            >,
            mi::ordered_non_unique<
                mi::tag<email_domain_tag>,
                mi::member<Account, std::string, &Account::email_domain>,
                string_view_compare
            >,
            mi::ordered_non_unique<
                mi::tag<status_tag>,
                mi::member<Account, Account::Status, &Account::status>
            >,
            mi::ordered_non_unique<
                mi::tag<first_name_tag>,
                mi::member<Account, Account::first_name_t, &Account::first_name>,
                string_view_compare
            >,
            mi::ordered_non_unique<
                mi::tag<second_name_tag>,
                mi::member<Account, Account::second_name_t, &Account::second_name>,
                string_view_compare
            >,
            mi::ordered_non_unique<
                mi::tag<phone_tag>,
                mi::member<Account, std::string, &Account::phone>,
                string_view_compare
            >,
            mi::ordered_non_unique<
                mi::tag<phone_code_tag>,
                mi::member<Account, std::string, &Account::phone_code>,
                string_view_compare
            >,
            mi::ordered_non_unique<
                mi::tag<sex_tag>,
                mi::member<Account, bool, &Account::is_male>
            >,
            mi::ordered_non_unique<
                mi::tag<birth_tag>,
                mi::member<Account, int32_t, &Account::birth>
            >,
            mi::ordered_non_unique<
                mi::tag<birth_year_tag>,
                mi::member<Account, uint16_t, &Account::birth_year>
            >,
            mi::ordered_non_unique<
                mi::tag<country_tag>,
                mi::member<Account, Account::country_t, &Account::country>,
                string_view_compare
            >,
            mi::ordered_non_unique<
                mi::tag<city_tag>,
                mi::member<Account, Account::city_t, &Account::city>,
                string_view_compare
            >,
            mi::ordered_non_unique<
                mi::tag<joined_tag>,
                mi::member<Account, int32_t, &Account::joined>
            >,
            mi::ordered_non_unique<
                mi::tag<premium_tag>,
                mi::member<Account, int32_t, &Account::premium_finish>
            >
        >
    > account;
    // *INDENT-ON*

    std::map<Account::interest_t, std::vector<ShortAccount>, string_view_compare> interest;
    std::map<uint32_t, std::vector<uint32_t>> liked_by;

    void add_account(Account &&new_account)
    {
        account.insert(std::move(new_account));
    }

    void build_indicies()
    {
        auto &index = account.get<id_tag>();
        for (auto &account : index)
        {
            for (auto &account_interest : account.interest)
            {
                ShortAccount short_account{account.id, account.email};
                interest[account_interest].push_back(std::move(short_account));
            }

            for (auto &like : account.like)
            {
                liked_by[like].push_back(account.id);
            }
        }

        for (auto &account : liked_by)
        {
            account.second.erase(std::unique(account.second.begin(), account.second.end()), account.second.end());
        }
    }
};
