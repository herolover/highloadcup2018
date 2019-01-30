#pragma once

#include "../RequestHandler.h"
#include "../AccountParser.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/reader.h>
#include <rapidjson/memorystream.h>

template<>
struct RequestHandler<UpdateAccount>
{
    static void handle(DB &db, UpdateAccount &request, HttpServer::HttpResponse &response)
    {
        thread_local rj::GenericReader<rj::UTF8<>, rj::UTF8<>> reader;

        DB::UpdateAccountResult result;
        AccountParser parser(db, [&db, &result, &request](Account &&account)
        {
            account.id = request.account_id;
            account.interest_mask = db.get_interest_mask(account.interest_list);
            result = db.update_account(std::move(account));
        }, AccountParserState::ACCOUNT_KEY);

        rapidjson::MemoryStream stream(request.body, request.size);
        auto is_parsed = reader.Parse(stream, parser);
        if (is_parsed && result == DB::UpdateAccountResult::SUCCESS)
        {
            response.result(boost::beast::http::status::accepted);
            response.body() = "{}";
        }
        else if (!is_parsed || result == DB::UpdateAccountResult::INVALID_ACCOUNT_DATA)
        {
            response.result(boost::beast::http::status::bad_request);
        }
        else
        {
            response.result(boost::beast::http::status::not_found);
        }
        response.prepare_payload();
    }
};
