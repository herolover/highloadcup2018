#pragma once

#include "../RequestHandler.h"

#include <codecvt>
#include <functional>

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

template<class Handler>
class LikeParser: public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, LikeParser<Handler>>
{
public:
    LikeParser(DB &db, Handler &&handler)
        : _db(db)
        , _handler(std::forward<Handler>(handler))
    {
    }

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
            if (value == "likes")
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
            const static std::map<std::string_view, LikeParserState> key_list =
            {
                {"likee", LikeParserState::LIKEE_ID},
                {"liker", LikeParserState::LIKER_ID},
                {"ts", LikeParserState::LIKE_TS},
            };

            auto key_list_it = key_list.find(value);
            if (key_list_it != key_list.end())
            {
                _state = key_list_it->second;
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
            _likee_id = value;
            if (!_db.has_account(_likee_id).first)
            {
                return false;
            }
            _state = LikeParserState::LIKE_KEY;
            break;
        case LikeParserState::LIKER_ID:
            _liker_id = value;
            if (!_db.has_account(_liker_id).first)
            {
                return false;
            }
            _state = LikeParserState::LIKE_KEY;
            break;
        case LikeParserState::LIKE_TS:
            _like_ts = value;
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
            if (_likee_id == 0 || _liker_id == 0 || _like_ts == 0)
            {
                return false;
            }
            _handler(_likee_id, _liker_id, _like_ts);
            _likee_id = 0;
            _liker_id = 0;
            _like_ts = 0;
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

private:
    DB &_db;
    Handler _handler;
    LikeParserState _state = LikeParserState::KEY;
    uint32_t _likee_id = 0;
    uint32_t _liker_id = 0;
    int32_t _like_ts = 0;
};

template<>
struct RequestHandler<AddLikes>
{
    static void handle(DB &db, AddLikes &request, HttpServer::HttpResponse &response)
    {
        thread_local rapidjson::GenericReader<rapidjson::UTF8<>, rapidjson::UTF8<>> reader;

        std::vector<std::tuple<uint32_t, uint32_t, int32_t>> like_list;
        LikeParser parser(db, [&like_list](uint32_t likee_id, uint32_t liker_id, int32_t like_ts)
        {
            like_list.emplace_back(likee_id, liker_id, like_ts);
        });
        rapidjson::MemoryStream stream(request.body, request.size);
        auto is_parsed = reader.Parse(stream, parser);
        if (is_parsed)
        {
            db.add_like_list(like_list);
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
