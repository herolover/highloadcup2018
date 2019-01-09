#pragma once

#include "Account.h"
#include "Convert.h"
#include "DB.h"

#include <rapidjson/reader.h>
#include <rapidjson/encodings.h>

#include <map>
#include <locale>
#include <codecvt>
#include <functional>

namespace rj = rapidjson;

class DBLoader: public rj::BaseReaderHandler<rj::UTF8<>, DBLoader>
{
public:
    enum class State
    {
        KEY = 0,
        ACCOUNT_KEY = 1,
        ID = 2,
        EMAIL = 3,
        FIRST_NAME = 4,
        SECOND_NAME = 5,
        PHONE = 6,
        SEX = 7,
        BIRTH = 8,
        COUNTRY = 9,
        CITY = 10,
        JOINED = 11,
        STATUS = 12,
        INTEREST = 13,
        PREMIUM_KEY = 14,
        PREMIUM_START = 15,
        PREMIUM_FINISH = 16,
        LIKE_KEY = 17,
        LIKE_ID = 18,
        LIKE_TS = 19
    };

    DBLoader(DB &db)
        : _db(db)
    {
    }

    bool String(const char *raw_value, rj::SizeType length, bool)
    {
        std::string_view value(raw_value, length);

        switch (state)
        {
        case State::KEY:
        {
            if (value == "accounts")
            {
                state = State::ACCOUNT_KEY;
            }
            else
            {
                return false;
            }
        }
        break;
        case State::ACCOUNT_KEY:
        {
            const static std::map<std::string_view, State> key_list =
            {
                {"id", State::ID},
                {"email", State::EMAIL},
                {"fname", State::FIRST_NAME},
                {"sname", State::SECOND_NAME},
                {"phone", State::PHONE},
                {"sex", State::SEX},
                {"birth", State::BIRTH},
                {"country", State::COUNTRY},
                {"city", State::CITY},
                {"joined", State::JOINED},
                {"status", State::STATUS},
                {"interests", State::INTEREST},
                {"premium", State::PREMIUM_KEY},
                {"likes", State::LIKE_KEY},
            };

            auto key_list_it = key_list.find(value);
            if (key_list_it != key_list.end())
            {
                state = key_list_it->second;
            }
            else
            {
                return false;
            }
        }
        break;
        case State::EMAIL:
            account.email = value;
            account.email_domain = account.email.substr(account.email.find('@') + 1);
            state = State::ACCOUNT_KEY;
            break;
        case State::FIRST_NAME:
            account.first_name = value;
            state = State::ACCOUNT_KEY;
            break;
        case State::SECOND_NAME:
            account.second_name = value;
            state = State::ACCOUNT_KEY;
            break;
        case State::PHONE:
            account.phone = value;
            account.phone_code = account.phone.substr(account.phone.find('(') + 1, 3);
            state = State::ACCOUNT_KEY;
            break;
        case State::SEX:
            account.is_male = convert_sex(value);
            state = State::ACCOUNT_KEY;
            break;
        case State::COUNTRY:
            account.country = value;
            state = State::ACCOUNT_KEY;
            break;
        case State::CITY:
            account.city = value;
            state = State::ACCOUNT_KEY;
            break;
        case State::STATUS:
            account.status = convert_account_status(value);
            state = State::ACCOUNT_KEY;
            break;
        case State::INTEREST:
            account.interest.push_back(value);
            break;
        case State::PREMIUM_KEY:
        {
            const static std::map<std::string_view, State> key_list =
            {
                {"start", State::PREMIUM_START},
                {"finish", State::PREMIUM_FINISH},
            };

            auto key_list_it = key_list.find(value);
            if (key_list_it != key_list.end())
            {
                state = key_list_it->second;
            }
            else
            {
                return false;
            }
        }
        break;
        case State::LIKE_KEY:
        {
            const static std::map<std::string_view, State> key_list =
            {
                {"id", State::LIKE_ID},
                {"ts", State::LIKE_TS},
            };

            auto key_list_it = key_list.find(value);
            if (key_list_it != key_list.end())
            {
                state = key_list_it->second;
            }
            else
            {
                return false;
            }
        }
        break;
        default:
            return false;
        }

        return true;
    }

    bool Int(int32_t value)
    {
        switch (state)
        {
        case DBLoader::State::BIRTH:
            account.birth = value;
            account.birth_year = get_year(value);
            state = State::ACCOUNT_KEY;
            break;
        default:
            return false;
        }

        return true;
    }

    bool Uint(uint32_t value)
    {
        switch (state)
        {
        case State::ID:
            account.id = value;
            state = State::ACCOUNT_KEY;
            break;
        case State::BIRTH:
            account.birth = value;
            account.birth_year = get_year(value);
            state = State::ACCOUNT_KEY;
            break;
        case State::JOINED:
            account.joined = value;
            account.joined_year = get_year(value);
            state = State::ACCOUNT_KEY;
            break;
        case State::PREMIUM_START:
            account.premium_start = value;
            state = State::PREMIUM_KEY;
            break;
        case State::PREMIUM_FINISH:
            account.premium_finish = value;
            state = State::PREMIUM_KEY;
            break;
        case State::LIKE_ID:
            //account.like.back().id = value;
            account.like.push_back(value);
            state = State::LIKE_KEY;
            break;
        case State::LIKE_TS:
            //account.like.back().ts = value;
            state = State::LIKE_KEY;
            break;
        default:
            return false;
        }

        return true;
    }

    bool StartObject()
    {
        if (state == State::LIKE_KEY)
        {
            //account.like.emplace_back();
        }

        return true;
    }

    bool EndObject(rj::SizeType)
    {
        if (state == State::PREMIUM_KEY)
        {
            state = State::ACCOUNT_KEY;
        }
        else if (state == State::ACCOUNT_KEY)
        {
            std::sort(account.interest.begin(), account.interest.end(), string_view_compare());
            std::sort(account.like.begin(), account.like.end());
            _db.add_account(std::move(account));
            account = {};
        }

        return true;
    }

    bool EndArray(rj::SizeType)
    {
        if (state == State::INTEREST || state == State::LIKE_KEY)
        {
            state = State::ACCOUNT_KEY;
        }
        else if (state == State::ACCOUNT_KEY)
        {
            state = State::KEY;
        }

        return true;
    }

private:
    State state = State::KEY;
    Account account;

    DB &_db;
};