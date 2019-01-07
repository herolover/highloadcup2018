#pragma once

#include <string>
#include <memory>
#include <set>

using string_ptr = std::shared_ptr<std::string>;

inline bool operator<(const std::string &a, const std::string_view &b)
{
    return std::string_view(a.c_str(), a.size()) < b;
}

inline bool operator<(const std::string_view &a, const std::string &b)
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
    bool operator()(const T &a, std::nullptr_t) const
    {
        return false;
    }

    template<class T>
    bool operator()(std::nullptr_t, const T &a) const
    {
        return a != nullptr;
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
    using Type = shared_string<ID>;

    shared_string() = default;
    shared_string(const shared_string &) = default;
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

    Type &operator=(const shared_string &) = default;
    Type &operator=(shared_string &&) = default;

    operator bool() const
    {
        return _value != nullptr;
    }

    const std::string &operator*() const
    {
        return *_value;
    }

    const std::string *operator->() const
    {
        return _value.get();
    }

    bool operator==(nullptr_t) const
    {
        return _value == nullptr;
    }

    bool operator!=(nullptr_t) const
    {
        return _value != nullptr;
    }

    bool operator==(const std::string_view &value) const
    {
        return std::string_view(_value->c_str(), _value->size()) == value;
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
