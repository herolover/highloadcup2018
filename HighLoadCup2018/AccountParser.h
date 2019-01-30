#pragma once

#include "DB.h"
#include "Account.h"
#include "Convert.h"

#include <rapidjson/reader.h>
#include <rapidjson/encodings.h>

#include <map>
#include <locale>
#include <codecvt>
#include <functional>
#include <bitset>

namespace rj = rapidjson;

enum class AccountParserState
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

template<class Handler>
class AccountParser: public rj::BaseReaderHandler<rj::UTF8<>, AccountParser<Handler>>
{
public:
    AccountParser(DB &db, Handler &&handler, AccountParserState state = AccountParserState::KEY)
        : _db(db)
        , _handler(std::forward<Handler>(handler))
        , _state(state)
    {
    }

    bool Default()
    {
        return false;
    }

    bool String(const char *raw_value, rj::SizeType length, bool)
    {
        std::string_view value(raw_value, length);

        switch (_state)
        {
        case AccountParserState::KEY:
        {
            if (value == "accounts")
            {
                _state = AccountParserState::ACCOUNT_KEY;
            }
            else
            {
                return false;
            }
        }
        break;
        case AccountParserState::ACCOUNT_KEY:
        {
            const static std::map<std::string_view, AccountParserState> key_list =
            {
                {"id", AccountParserState::ID},
                {"email", AccountParserState::EMAIL},
                {"fname", AccountParserState::FIRST_NAME},
                {"sname", AccountParserState::SECOND_NAME},
                {"phone", AccountParserState::PHONE},
                {"sex", AccountParserState::SEX},
                {"birth", AccountParserState::BIRTH},
                {"country", AccountParserState::COUNTRY},
                {"city", AccountParserState::CITY},
                {"joined", AccountParserState::JOINED},
                {"status", AccountParserState::STATUS},
                {"interests", AccountParserState::INTEREST},
                {"premium", AccountParserState::PREMIUM_KEY},
                {"likes", AccountParserState::LIKE_KEY},
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
        case AccountParserState::EMAIL:
        {
            _account.email = value;
            auto at_pos = _account.email.find('@');
            if (at_pos != std::string::npos)
            {
                _account.email_domain = _account.email.substr(at_pos + 1);
            }
            else
            {
                return false;
            }
            _state = AccountParserState::ACCOUNT_KEY;
            break;
        }
        case AccountParserState::FIRST_NAME:
            _account.first_name = value;
            _state = AccountParserState::ACCOUNT_KEY;
            break;
        case AccountParserState::SECOND_NAME:
            _account.second_name = value;
            _state = AccountParserState::ACCOUNT_KEY;
            break;
        case AccountParserState::PHONE:
            _account.phone = value;
            _account.phone_code = _account.phone.substr(_account.phone.find('(') + 1, 3);
            _state = AccountParserState::ACCOUNT_KEY;
            break;
        case AccountParserState::SEX:
            if (value.size() != 1)
            {
                return false;
            }
            _account.sex = convert_sex(value);
            if (_account.sex == Account::Sex::INVALID)
            {
                return false;
            }
            _state = AccountParserState::ACCOUNT_KEY;
            break;
        case AccountParserState::COUNTRY:
            _account.country = value;
            _state = AccountParserState::ACCOUNT_KEY;
            break;
        case AccountParserState::CITY:
            _account.city = value;
            _state = AccountParserState::ACCOUNT_KEY;
            break;
        case AccountParserState::STATUS:
            _account.status = convert_account_status(value);
            if (_account.status == Account::Status::INVALID)
            {
                return false;
            }
            _state = AccountParserState::ACCOUNT_KEY;
            break;
        case AccountParserState::INTEREST:
            _account.add_interest(_db.add_interest(value));
            break;
        case AccountParserState::PREMIUM_KEY:
        {
            const static std::map<std::string_view, AccountParserState> key_list =
            {
                {"start", AccountParserState::PREMIUM_START},
                {"finish", AccountParserState::PREMIUM_FINISH},
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
        case AccountParserState::LIKE_KEY:
        {
            const static std::map<std::string_view, AccountParserState> key_list =
            {
                {"id", AccountParserState::LIKE_ID},
                {"ts", AccountParserState::LIKE_TS},
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
        case AccountParserState::BIRTH:
            _account.birth = value;
            _account.birth_year = get_year(value);
            _state = AccountParserState::ACCOUNT_KEY;
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
        case AccountParserState::ID:
            _account.id = value;
            _state = AccountParserState::ACCOUNT_KEY;
            break;
        case AccountParserState::BIRTH:
            _account.birth = value;
            _account.birth_year = get_year(value);
            _state = AccountParserState::ACCOUNT_KEY;
            break;
        case AccountParserState::JOINED:
            _account.joined = value;
            _account.joined_year = get_year(value);
            _state = AccountParserState::ACCOUNT_KEY;
            break;
        case AccountParserState::PREMIUM_START:
            _account.premium_start = value;
            _state = AccountParserState::PREMIUM_KEY;
            break;
        case AccountParserState::PREMIUM_FINISH:
            _account.premium_finish = value;
            _state = AccountParserState::PREMIUM_KEY;
            break;
        case AccountParserState::LIKE_ID:
            _like_id = value;
            _state = AccountParserState::LIKE_KEY;
            break;
        case AccountParserState::LIKE_TS:
            _like_ts = value;
            _state = AccountParserState::LIKE_KEY;
            break;
        default:
            return false;
        }

        return true;
    }

    bool StartObject()
    {
        return _state == AccountParserState::KEY || _state == AccountParserState::ACCOUNT_KEY || _state == AccountParserState::LIKE_KEY || _state == AccountParserState::PREMIUM_KEY;
    }

    bool EndObject(rj::SizeType)
    {
        if (_state == AccountParserState::LIKE_KEY)
        {
            if (_like_id != 0 && _like_ts != 0)
            {
                _account.add_like(_like_id, _like_ts);
            }
            else
            {
                return false;
            }
        }
        else if (_state == AccountParserState::PREMIUM_KEY)
        {
            if (_account.premium_start != 0 && _account.premium_finish != 0)
            {
                _account.premium_status = _db.current_time > _account.premium_start && _db.current_time < _account.premium_finish ? Account::PremiumStatus::ACTIVE : Account::PremiumStatus::EXPIRED;
            }
            else if (_account.premium_start == _account.premium_finish)
            {
                _account.premium_status = Account::PremiumStatus::NO;
            }
            else
            {
                return false;
            }
            _state = AccountParserState::ACCOUNT_KEY;
        }
        else if (_state == AccountParserState::ACCOUNT_KEY)
        {
            _handler(std::move(_account));
            _account = {};
            _like_id = 0;
            _like_ts = 0;
        }

        return true;
    }

    bool StartArray()
    {
        return _state == AccountParserState::ACCOUNT_KEY || _state == AccountParserState::INTEREST || _state == AccountParserState::LIKE_KEY;
    }

    bool EndArray(rj::SizeType)
    {
        if (_state == AccountParserState::INTEREST || _state == AccountParserState::LIKE_KEY)
        {
            _state = AccountParserState::ACCOUNT_KEY;
        }
        else if (_state == AccountParserState::ACCOUNT_KEY)
        {
            _state = AccountParserState::KEY;
        }

        return true;
    }

private:
    DB &_db;
    Handler _handler;
    AccountParserState _state = AccountParserState::KEY;
    Account _account;
    uint32_t _like_id = 0;
    int32_t _like_ts = 0;
};