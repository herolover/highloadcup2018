#pragma once

template<class iter_type, class Handler>
class handler_iter
{
public:

    handler_iter(iter_type it, Handler &&handler)
        : _it(it)
        , _handler(handler)
    {
        _handler(_it);
    }

    auto operator*() const
    {
        return *_it;
    }

    auto operator->() const
    {
        return _it.operator->();
    }

    bool operator!=(const iter_type &another_it) const
    {
        return _it != another_it;
    }

    handler_iter &operator++()
    {
        ++_it;
        _handler(_it);

        return *this;
    }

private:
    iter_type _it;
    Handler _handler;
};