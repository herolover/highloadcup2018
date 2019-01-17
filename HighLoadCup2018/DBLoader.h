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
        _current_time = std::time(nullptr);
    }

    bool String(const char *raw_value, rj::SizeType length, bool)
    {
        std::string_view value(raw_value, length);

        switch (_state)
        {
        case State::KEY:
        {
            if (value == "accounts")
            {
                _state = State::ACCOUNT_KEY;
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
                _state = key_list_it->second;
            }
            else
            {
                return false;
            }
        }
        break;
        case State::EMAIL:
            _account.email = value;
            _account.email_domain = _account.email.substr(_account.email.find('@') + 1);
            _state = State::ACCOUNT_KEY;
            break;
        case State::FIRST_NAME:
            _account.first_name = value;
            _state = State::ACCOUNT_KEY;
            break;
        case State::SECOND_NAME:
            _account.second_name = value;
            _state = State::ACCOUNT_KEY;
            break;
        case State::PHONE:
            _account.phone = value;
            _account.phone_code = _account.phone.substr(_account.phone.find('(') + 1, 3);
            _state = State::ACCOUNT_KEY;
            break;
        case State::SEX:
            _account.is_male = convert_sex(value);
            _state = State::ACCOUNT_KEY;
            break;
        case State::COUNTRY:
            _account.country = value;
            _state = State::ACCOUNT_KEY;
            break;
        case State::CITY:
            _account.city = value;
            _state = State::ACCOUNT_KEY;
            break;
        case State::STATUS:
            _account.status = convert_account_status(value);
            _state = State::ACCOUNT_KEY;
            break;
        case State::INTEREST:
            _account.add_interest(_db.add_interest(value));
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
                _state = key_list_it->second;
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
                _state = key_list_it->second;
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
        switch (_state)
        {
        case DBLoader::State::BIRTH:
            _account.birth = value;
            _account.birth_year = get_year(value);
            _state = State::ACCOUNT_KEY;
            break;
        default:
            return false;
        }

        return true;
    }

    bool Uint(uint32_t value)
    {
        switch (_state)
        {
        case State::ID:
            _account.id = value;
            _state = State::ACCOUNT_KEY;
            break;
        case State::BIRTH:
            _account.birth = value;
            _account.birth_year = get_year(value);
            _state = State::ACCOUNT_KEY;
            break;
        case State::JOINED:
            _account.joined = value;
            _account.joined_year = get_year(value);
            _state = State::ACCOUNT_KEY;
            break;
        case State::PREMIUM_START:
            _account.premium_start = value;
            _state = State::PREMIUM_KEY;
            break;
        case State::PREMIUM_FINISH:
            _account.premium_finish = value;
            _state = State::PREMIUM_KEY;
            break;
        case State::LIKE_ID:
            _like_id = value;
            _state = State::LIKE_KEY;
            break;
        case State::LIKE_TS:
            _like_ts = value;
            _state = State::LIKE_KEY;
            break;
        default:
            return false;
        }

        return true;
    }

    bool EndObject(rj::SizeType)
    {
        if (_state == State::LIKE_KEY)
        {
            _account.add_like(_like_id, _like_ts);
        }
        else if (_state == State::PREMIUM_KEY)
        {
            if (_account.premium_start != 0 && _account.premium_finish != 0)
            {
                _account.premium_status = _current_time > _account.premium_start && _current_time < _account.premium_finish ? Account::PremiumStatus::ACTIVE : Account::PremiumStatus::EXPIRED;
            }
            _state = State::ACCOUNT_KEY;
        }
        else if (_state == State::ACCOUNT_KEY)
        {
            _db.add_account(std::move(_account));
            _account = {};
        }

        return true;
    }

    bool EndArray(rj::SizeType)
    {
        if (_state == State::INTEREST || _state == State::LIKE_KEY)
        {
            _state = State::ACCOUNT_KEY;
        }
        else if (_state == State::ACCOUNT_KEY)
        {
            _state = State::KEY;
        }

        return true;
    }

private:
    State _state = State::KEY;
    Account _account;
    uint32_t _like_id;
    int32_t _like_ts;

    DB &_db;
    std::time_t _current_time;
};