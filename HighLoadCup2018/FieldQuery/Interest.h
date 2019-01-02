#pragma once

#include "../FieldQuery.h"

#include "../Split.h"

template<>
struct FieldQuery<DB::interest_tag>
{
    static auto contains(DB &db, const std::string_view &value)
    {
        bool is_inited = false;
        std::vector<uint32_t> result_id_list;

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
                result_id_list.erase(std::remove_if(result_id_list.begin(), result_id_list.end(), [&](uint32_t id)
                {
                    return std::find(id_list.begin(), id_list.end(), id) == id_list.end();
                }), result_id_list.end());
            }
        }

        return result_id_list;
    }

    static auto any(DB &db, const std::string_view &value)
    {
        std::vector<uint32_t> result_id_list;

        for (auto &interest : split(value))
        {
            auto &id_list = db.interest[interest];
            result_id_list.insert(result_id_list.end(), id_list.begin(), id_list.end());
        }

        return result_id_list;
    }

    template<class Iter>
    static bool filter(DB &db, const std::string_view &method, const std::string_view &value, Iter result_begin, Iter result_end)
    {
        std::vector<uint32_t> method_result;
        if (method == std::string_view("contains"))
        {
            method_result = contains(db, value);
        }
        else if (method == std::string_view("any"))
        {
            method_result = any(db, value);
        }
        else
        {
            return false;
        }

        std::remove_if(result_begin, result_end, [](uint32_t id)
        {
            return std::find(method_result.begin(), method_result.end(), id) == method_result.end();
        });

        return true;
    }
};
