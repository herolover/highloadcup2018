#pragma once

#include "../RequestHandler.h"

#include <codecvt>
#include <functional>
#include <array>

#include <rapidjson/rapidjson.h>
#include <rapidjson/reader.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/reader.h>
#include <rapidjson/encodings.h>

enum class LikeParserState
{
    KEY = 0,
    LIKE_KEY = 1,
    LIKEE_ID = 2,
    LIKER_ID = 3,
    LIKE_TS = 4
};

class LikeParser: public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, LikeParser>
{
public:
    bool Default()
    {
        return false;
    }

    bool String(const char *raw_value, rapidjson::SizeType length, bool)
    {
        std::string_view value(raw_value, length);

        switch (_state)
        {
        case LikeParserState::KEY:
        {
            if (value == "likes"sv)
            {
                _state = LikeParserState::LIKE_KEY;
            }
            else
            {
                return false;
            }
        }
        break;
        case LikeParserState::LIKE_KEY:
        {
            if (value == "likee"sv)
            {
                _state = LikeParserState::LIKEE_ID;
            }
            else if (value == "liker"sv)
            {
                _state = LikeParserState::LIKER_ID;
            }
            else if (value == "ts"sv)
            {
                _state = LikeParserState::LIKE_TS;
            }
            else
            {
                return false;
            }
        }
        break;
        default:
            return false;
        }

        return true;
    }

    bool Uint(uint32_t value)
    {
        switch (_state)
        {
        case LikeParserState::LIKEE_ID:
            _like_list[_like_list_size].likee_id = value;
            _state = LikeParserState::LIKE_KEY;
            break;
        case LikeParserState::LIKER_ID:
            _like_list[_like_list_size].liker_id = value;
            _state = LikeParserState::LIKE_KEY;
            break;
        case LikeParserState::LIKE_TS:
            _like_list[_like_list_size].like_ts = value;
            _state = LikeParserState::LIKE_KEY;
            break;
        default:
            return false;
        }

        return true;
    }

    bool StartObject()
    {
        return _state == LikeParserState::KEY || _state == LikeParserState::LIKE_KEY;
    }

    bool EndObject(rapidjson::SizeType)
    {
        if (_state == LikeParserState::LIKE_KEY)
        {
            if (_like_list[_like_list_size].likee_id == 0 || _like_list[_like_list_size].liker_id == 0 || _like_list[_like_list_size].like_ts == 0)
            {
                return false;
            }
            ++_like_list_size;
        }
        else if (_state != LikeParserState::KEY)
        {
            return false;
        }

        return true;
    }

    bool StartArray()
    {
        return _state == LikeParserState::LIKE_KEY;
    }

    bool EndArray(rapidjson::SizeType)
    {
        if (_state == LikeParserState::LIKE_KEY)
        {
            _state = LikeParserState::KEY;

            return true;
        }
        else
        {
            return false;
        }
    }

    const auto &like_list() const
    {
        return _like_list;
    }

    std::size_t like_list_size() const
    {
        return _like_list_size;
    }

private:
    LikeParserState _state = LikeParserState::KEY;
    std::array<NewLike, 150> _like_list;
    std::size_t _like_list_size = 0;
};

template<>
struct RequestHandler<AddLikes>
{
    static void handle(DB &db, AddLikes &request, HttpServer::HttpResponse &response)
    {
        thread_local rapidjson::GenericReader<rapidjson::UTF8<>, rapidjson::UTF8<>> reader;

        LikeParser parser;
        rapidjson::MemoryStream stream(request.body, request.size);
        auto is_parsed = reader.Parse(stream, parser);
        if (is_parsed && db.add_like_list(parser.like_list(), parser.like_list_size()))
        {
            response.result(boost::beast::http::status::accepted);
            response.body() = "{}";
        }
        else
        {
            response.result(boost::beast::http::status::bad_request);
        }
        response.prepare_payload();
    }
};
