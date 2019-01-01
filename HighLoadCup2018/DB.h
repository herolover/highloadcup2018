#pragma once

#include "Account.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include <set>

namespace mi = boost::multi_index;

class DB
{
public:
    struct email_tag {};
    struct email_domain_tag {};
    struct status_tag {};
    struct first_name_tag {};
    struct second_name_tag {};
    struct phone_tag {};
    struct sex_tag {};
    struct birth_tag {};
    struct country_tag {};
    struct city_tag {};
    struct joined_tag {};

    // *INDENT-OFF*
    mi::multi_index_container<Account,
        mi::indexed_by<
            mi::ordered_unique<mi::identity<Account>>,
            mi::ordered_unique<
                mi::tag<email_tag>,
                mi::member<Account, std::string, &Account::email>,
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
                mi::tag<sex_tag>,
                mi::member<Account, bool, &Account::is_male>
            >,
            mi::ordered_non_unique<
                mi::tag<birth_tag>,
                mi::member<Account, uint32_t, &Account::birth>
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
                mi::member<Account, uint32_t, &Account::joined>
            >
        >
    > account;
    // *INDENT-ON*
};