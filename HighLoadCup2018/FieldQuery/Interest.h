#pragma once

#include "../FieldQuery.h"

#include "../Split.h"

template<>
struct FieldQuery<DB::interest_tag>
{
    static auto contains(DB &db, const std::string_view &value)
    {
        bool is_inited = false;
        std::set<uint32_t> result_id_list;

        for (auto &interest : split(value))
        {
            auto &id_list = db.interest[interest];
            if (!is_inited)
            {
                result_id_list = id_list;
                is_inited = true;
            }
            else
            {
                for (auto it = result_id_list.begin(); it != result_id_list.end();)
                {
                    if (id_list.find(*it) == id_list.end())
                    {
                        it = result_id_list.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }
        }

        return result_id_list;
    }

    static auto any(DB &db, const std::string_view &value)
    {
        std::set<uint32_t> result_id_list;

        for (auto &interest : split(value))
        {
            result_id_list.merge(db.interest[interest]);
        }

        return result_id_list;
    }
};
