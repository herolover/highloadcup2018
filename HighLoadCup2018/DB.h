#pragma once

#include "Account.h"

#include <boost/bimap.hpp>

class DB
{
public:
    std::vector<Account> account;

#define DEFINE_ENTITY(entity, id_type) \
public: \
    ##id_type get_##entity##_id(const std::wstring &value) \
    { \
        auto it = _##entity.left.find(value); \
        if (it != _##entity.left.end()) \
        { \
            return it->second; \
        } \
        else \
        { \
            ##id_type id = static_cast<##id_type>(_##entity.size()); \
            _##entity.insert({value, id}); \
\
            return id; \
        } \
    } \
\
    auto entity##_size() const \
    { \
        return _##entity##.size(); \
    } \
private: \
    boost::bimap<std::wstring, ##id_type> _##entity

    DEFINE_ENTITY(first_name, int8_t);
    DEFINE_ENTITY(second_name, int16_t);
    DEFINE_ENTITY(country, int8_t);
    DEFINE_ENTITY(city, int16_t);
    DEFINE_ENTITY(interest, uint8_t);
#undef DEFINE_ENTITY
};
