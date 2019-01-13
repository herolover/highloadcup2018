#pragma once

#include <string>
#include <memory>
#include <set>

struct common_less
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

    bool operator==(const shared_string &a) const
    {
        return !_value && !a._value || _value && a._value && *_value == *a._value;
    }

    bool operator!=(const shared_string &a) const
    {
        return !(*this == a);
    }

    bool operator<(const shared_string &a) const
    {
        return common_less()(*this, a);
    }

    bool operator==(const std::string_view &value) const
    {
        return _value && *_value == value;
    }

    bool operator!=(const std::string_view &value) const
    {
        return !(*this != value);
    }

    bool operator<(const std::string_view &value) const
    {
        return common_less()(*this, value);
    }

    static std::size_t size()
    {
        return _value_storage.size();
    }

private:
    std::shared_ptr<std::string> _value;
    static std::set<std::shared_ptr<std::string>, common_less> _value_storage;
};

template<int ID>
std::set<std::shared_ptr<std::string>, common_less> shared_string<ID>::_value_storage;
