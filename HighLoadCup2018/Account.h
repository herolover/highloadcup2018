#pragma once

#include <string>
#include <vector>

struct Like
{
    uint32_t id;
    uint32_t ts;
};

struct Account
{
    enum class Status
    {
        FREE = 0,
        BUSY = 1,
        COMPLICATED = 2
    };

    uint32_t id;
    std::string email;
    std::wstring first_name;
    std::wstring second_name;
    std::wstring phone;
    bool is_male;
    uint32_t birth;
    std::wstring country;
    std::wstring city;
    uint32_t joined;
    Status status;
    std::vector<std::wstring> interests;
    uint32_t premium_start;
    uint32_t premium_finish;
    std::vector<Like> likes;

    bool operator<(const Account &a) const
    {
        return id < a.id;
    }
};

bool is_male(const wchar_t *value)
{
    return value[0] == L'm';
}

Account::Status convert_account_status(const wchar_t *value)
{
    if (std::wcscmp(value, L"свободны") == 0)
    {
        return Account::Status::FREE;
    }
    else if (std::wcscmp(value, L"заняты") == 0)
    {
        return Account::Status::BUSY;
    }
    else
    {
        return Account::Status::COMPLICATED;
    }
}