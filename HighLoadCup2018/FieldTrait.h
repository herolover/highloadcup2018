#pragma once

#include "DB.h"

#include <set>

enum class AccountField
{
    ID,
    EMAIL,
    FIRST_NAME,
    SECOND_NAME,
    PHONE,
    SEX,
    BIRTH,
    COUNTRY,
    CITY,
    JOINED,
    STATUS,
    INTEREST,
    PREMIUM,
    LIKE
};

template<AccountField Field>
struct GetTrait
{
};
