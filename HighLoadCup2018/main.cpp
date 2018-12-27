//#include <boost/asio.hpp>
#include <boost/multi_index_container.hpp>

#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/encodings.h>

#include <string>
#include <cstdio>
#include <map>
#include <iostream>

namespace mi = boost::multi_index;
namespace rj = rapidjson;

struct Like
{
    uint32_t id;
    uint32_t ts;
};

struct Account
{
    uint32_t id;
    std::wstring email;
    std::wstring first_name;
    std::wstring second_name;
    std::wstring phone;
    std::wstring sex;
    uint32_t birth;
    std::wstring country;
    std::wstring city;
    uint32_t joined;
    std::wstring status;
    std::vector<std::wstring> interests;
    uint32_t premium_start;
    uint32_t premium_finish;
    std::vector<Like> likes;

    bool operator<(const Account &a) const
    {
        return id < a.id;
    }
};

struct AccountReaderHandler: public rj::BaseReaderHandler<rj::UTF16<>, AccountReaderHandler>
{
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

    State state = State::KEY;
    Account account;
    std::vector<Account> accounts;

    bool String(const wchar_t *value, rj::SizeType length, bool)
    {
        switch (state)
        {
        case State::KEY:
        {
            if (std::wcscmp(value, L"accounts") == 0)
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
            const static std::map<std::wstring, State> key_list =
            {
                {L"id", State::ID},
                {L"email", State::EMAIL},
                {L"fname", State::FIRST_NAME},
                {L"sname", State::SECOND_NAME},
                {L"phone", State::PHONE},
                {L"sex", State::SEX},
                {L"birth", State::BIRTH},
                {L"country", State::COUNTRY},
                {L"city", State::CITY},
                {L"joined", State::JOINED},
                {L"status", State::STATUS},
                {L"interests", State::INTEREST},
                {L"premium", State::PREMIUM_KEY},
                {L"likes", State::LIKE_KEY},
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
            state = State::ACCOUNT_KEY;
            break;
        case State::SEX:
            account.sex = value;
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
            account.status = value;
            state = State::ACCOUNT_KEY;
            break;
        case State::INTEREST:
            account.interests.push_back(value);
            break;
        case State::PREMIUM_KEY:
        {
            const static std::map<std::wstring, State> key_list =
            {
                {L"start", State::PREMIUM_START},
                {L"finish", State::PREMIUM_FINISH},
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
            const static std::map<std::wstring, State> key_list =
            {
                {L"id", State::LIKE_ID},
                {L"ts", State::LIKE_TS},
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
            state = State::ACCOUNT_KEY;
            break;
        case State::JOINED:
            account.joined = value;
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
            account.likes.back().id = value;
            state = State::LIKE_KEY;
            break;
        case State::LIKE_TS:
            account.likes.back().ts = value;
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
            account.likes.emplace_back();
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
            accounts.push_back(std::move(account));
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
};

int main()
{
    // *INDENT-OFF*
    //mi::multi_index_container<Account,
    //    mi::indexed_by<
    //        mi::ordered_unique<
    //            mi::identity<Account>>>> account_list;
    // *INDENT-ON*

    AccountReaderHandler reader_handler;
    {
        rj::Document json;
        rj::GenericReader<rj::UTF8<>, rj::UTF16<>> reader;
        std::FILE *fp = std::fopen("accounts_3.json", "r");
        char buffer[1024];
        rj::FileReadStream file_stream(fp, buffer, 1024);
        reader.Parse(file_stream, reader_handler);
        std::fclose(fp);
    }

    std::cout << "Finished: " << (reader_handler.accounts.size()) << std::endl;

    return 0;
}