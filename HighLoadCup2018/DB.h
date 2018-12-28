#pragma once

#include "Account.h"

#include <boost/bimap.hpp>
#include <boost/multi_index_container.hpp>

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
                mi::member<Account, std::string, &Account::email>
            >,
            mi::ordered_non_unique<
                mi::tag<email_domain_tag>,
                mi::member<Account, std::string, &Account::email_domain>
            >,
            mi::ordered_non_unique<
                mi::tag<status_tag>,
                mi::member<Account, Account::Status, &Account::status>
            >,
            mi::ordered_non_unique<
                mi::tag<first_name_tag>,
                mi::member<Account, int8_t, &Account::first_name_id>
            >,
            mi::ordered_non_unique<
                mi::tag<second_name_tag>,
                mi::member<Account, int16_t, &Account::second_name_id>
            >,
            mi::ordered_non_unique<
                mi::tag<phone_tag>,
                mi::member<Account, std::string, &Account::phone>
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
                mi::member<Account, int8_t, &Account::country_id>
            >,
            mi::ordered_non_unique<
                mi::tag<city_tag>,
                mi::member<Account, int16_t, &Account::city_id>
            >,
            mi::ordered_non_unique<
                mi::tag<joined_tag>,
                mi::member<Account, uint32_t, &Account::joined>
            >
        >
    > account;
    // *INDENT-ON*

#define DEFINE_ENTITY(entity, id_type) \
public: \
    template<class CharType> \
    ##id_type get_##entity##_id(std::basic_string_view<CharType> value) \
    { \
        auto it = ##entity.left.find(std::wstring(value)); \
        if (it != ##entity.left.end()) \
        { \
            return it->second; \
        } \
        else \
        { \
            ##id_type id = static_cast<##id_type>(##entity.size()); \
            ##entity.insert({std::wstring(value), id}); \
\
            return id; \
        } \
    } \
\
    auto entity##_size() const \
    { \
        return entity##.size(); \
    } \
\
    boost::bimap<std::wstring, ##id_type> ##entity

    DEFINE_ENTITY(first_name, int8_t);
    DEFINE_ENTITY(second_name, int16_t);
    DEFINE_ENTITY(country, int8_t);
    DEFINE_ENTITY(city, int16_t);
    DEFINE_ENTITY(interest, uint8_t);
#undef DEFINE_ENTITY
};
