#pragma once

#include "Account.h"

#include <boost/bimap.hpp>
#include <boost/multi_index_container.hpp>

namespace mi = boost::multi_index;

class DB
{
public:
    struct first_name_tag
    {
    };

    // *INDENT-OFF*
    mi::multi_index_container<Account,
        mi::indexed_by<
            mi::ordered_unique<mi::identity<Account>>,
            mi::ordered_non_unique<
                mi::tag<first_name_tag>,
                mi::member<Account, int8_t, &Account::first_name_id>
            >
        >
    > account;
    // *INDENT-ON*

#define DEFINE_ENTITY(entity, id_type) \
public: \
    ##id_type get_##entity##_id(const std::wstring &value) \
    { \
        auto it = ##entity.left.find(value); \
        if (it != ##entity.left.end()) \
        { \
            return it->second; \
        } \
        else \
        { \
            ##id_type id = static_cast<##id_type>(##entity.size()); \
            ##entity.insert({value, id}); \
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
