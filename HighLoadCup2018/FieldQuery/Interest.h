#pragma once

#include "../FieldQuery.h"

#include "../MakeHandlerIter.h"
#include "../Common.h"
#include "../Split.h"

template<>
struct FieldQuery<DB::interest_tag>
{
    //static auto contains(DB &db, const std::string_view &value)
    //{
    //    bool is_inited = false;
    //    std::vector<uint32_t> result_id_list;

    //    for (auto &interest : split(value))
    //    {
    //        auto &id_list = db.interest[interest];
    //        if (!is_inited)
    //        {
    //            result_id_list = id_list;
    //            is_inited = true;
    //        }
    //        else
    //        {
    //            result_id_list.erase(std::remove_if(result_id_list.begin(), result_id_list.end(), [&](uint32_t id)
    //            {
    //                return std::find(id_list.begin(), id_list.end(), id) == id_list.end();
    //            }), result_id_list.end());
    //        }
    //    }

    //    return result_id_list;
    //}

    static auto any(DB &db, const std::string_view &value)
    {
        using IterType = std::vector<ShortAccount>::iterator;
        std::vector<std::pair<IterType, IterType>> range_list;

        for (auto &interest : split(value))
        {
            auto &id_list = db.interest[interest];
            range_list.push_back(std::make_pair(id_list.begin(), id_list.end()));
        }

        return std::make_pair(make_union_iter<false>(std::move(range_list), IterType()), IterType());
    }
};
