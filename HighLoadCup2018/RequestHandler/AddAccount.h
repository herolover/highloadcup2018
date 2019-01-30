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
        thread_local rj::GenericReader<rj::UTF8<>, rj::UTF8<>> reader;

        bool is_valid = false;
        AccountParser parser(db, [&db, &is_valid](Account &&account)
        {
            if (account.id != 0
                    && !account.email.empty()
                    && account.sex != Account::Sex::INVALID
                    && account.status != Account::Status::INVALID)
            {
                account.interest_mask = db.get_interest_mask(account.interest_list);
                is_valid = db.add_account(std::move(account));
            }
        }, AccountParserState::ACCOUNT_KEY);

        rapidjson::MemoryStream stream(request.body, request.size);
        auto is_parsed = reader.Parse(stream, parser);
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
