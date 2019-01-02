#pragma once

#include "../FieldQuery.h"

#include "../Split.h"

#include <charconv>
#include <algorithm>

template<>
struct FieldQuery<DB::like_tag>
{
    static auto contains(DB &db, const std::string_view &value)
    {
        bool is_inited = false;
        std::vector<uint32_t> result_id_list;

        for (auto &id_string : split(value))
        {
            uint32_t id = 0;
            std::from_chars(id_string.data(), id_string.data() + id_string.size(), id);

            auto &id_list = db.liked_by[id];
            if (!is_inited)
            {
                result_id_list = id_list;
                is_inited = true;
            }
            else
            {
                result_id_list.erase(std::remove_if(result_id_list.begin(), result_id_list.end(), [&](uint32_t id)
                {
                    return !std::binary_search(id_list.begin(), id_list.end(), id);
                }), result_id_list.end());
            }
        }

        return result_id_list;
    }
};
