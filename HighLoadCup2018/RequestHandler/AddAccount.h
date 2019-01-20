#pragma once

#include "../RequestHandler.h"
#include "../AccountParser.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/reader.h>
#include <rapidjson/memorystream.h>

template<>
struct RequestHandler<AddAccount>
{
    static void handle(DB &db, AddAccount &request, HttpServer::HttpResponse &response)
    {
        bool is_valid = false;
        static AccountParser parser(db, [&db, &is_valid](Account &&account)
        {
            if (account.id != 0
                    && !account.email.empty()
                    && account.sex != Account::Sex::INVALID
                    && account.status != Account::Status::INVALID
                    && account.premium_status != Account::PremiumStatus::INVALID)
            {
                is_valid = db.add_account(std::move(account));
            }
        }, AccountParserState::ACCOUNT_KEY);
        static rj::GenericReader<rj::UTF8<>, rj::UTF8<>> reader;

        parser.reset(AccountParserState::ACCOUNT_KEY);
        rapidjson::MemoryStream stream(request.body, request.size);
        auto is_parsed = reader.Parse(stream, parser);
        if (!is_parsed)
        {
            parser.reset(AccountParserState::ACCOUNT_KEY);
        }
        if (is_parsed && is_valid)
        {
            response.result(boost::beast::http::status::created);
            response.body() = "{}";
        }
        else
        {
            response.result(boost::beast::http::status::bad_request);
        }
        response.prepare_payload();
    }
};
