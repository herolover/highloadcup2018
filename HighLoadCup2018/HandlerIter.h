#pragma once

template<class ForwardIt, class Handler>
class handler_iter
{
public:
    handler_iter(ForwardIt it, Handler &&handler)
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

    bool operator!=(const ForwardIt &another_it) const
    {
        return _it != another_it;
    }

    handler_iter &operator++()
    {
        ++_it;
        _handler(_it);

        return *this;
    }

    handler_iter &operator--()
    {
        --_it;
        _handler(_it);

        return *this;
    }

private:
    ForwardIt _it;
    Handler _handler;
};
