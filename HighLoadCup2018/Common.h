#pragma once

#include <boost/iterator.hpp>

template<class RangeType>
auto make_reverse_range(const RangeType &range)
{
    return std::make_pair(boost::make_reverse_iterator(range.second), boost::make_reverse_iterator(range.first));
}
