#pragma once

#include <string>
#include <memory>
#include <set>

using string_ptr = std::shared_ptr<std::string>;

bool operator<(const std::string &a, const std::string_view &b)
{
    return std::string_view(a.c_str(), a.size()) < b;
}

bool operator<(const std::string_view &a, const std::string &b)
{
    return a < std::string_view(b.c_str(), b.size());
}

struct string_view_compare
{
    using is_transparent = void;

    template<class T>
    bool operator()(const T &a, const T &b) const
    {
        return !a && b || b && *a < *b;
    }

    template<class T>
    bool operator()(const T &a, const std::string_view &b) const
    {
        return !a || *a < b;
    }

    template<class T>
    bool operator()(const std::string_view &a, const T &b) const
    {
        return b && a < *b;
    }

    bool operator()(const std::string &a, const std::string &b) const
    {
        return a < b;
    }

    bool operator()(const std::string &a, const std::string_view &b) const
    {
        return a < b;
    }

    bool operator()(const std::string_view &a, const std::string &b) const
    {
        return a < b;
    }
};

template<int ID>
class shared_string
{
public:
    using Type = typename shared_string<ID>;

    shared_string() = default;
    shared_string(const std::string_view &value)
    {
        *this = value;
    }

    Type &operator=(const std::string_view &value)
    {
        auto it = _value_storage.find(value);
        if (it != _value_storage.end())
        {
            _value = *it;
        }
        else
        {
            _value = std::make_shared<std::string>(value);
            _value_storage.insert(_value);
        }

        return *this;
    }

    operator bool() const
    {
        return _value != nullptr;
    }

    const std::string &operator*() const
    {
        return *_value;
    }

    static std::size_t size()
    {
        return _value_storage.size();
    }

private:
    string_ptr _value;
    static std::set<string_ptr, string_view_compare> _value_storage;
};

template<int ID>
std::set<string_ptr, string_view_compare> shared_string<ID>::_value_storage;