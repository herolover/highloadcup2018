#pragma once

#include "../RequestHandler.h"

template<>
struct RequestHandler<BadRequest>
{
    static void handle(DB &db, const BadRequest &request, HttpServer::HttpResponse &response)
    {
        response.result(boost::beast::http::status::bad_request);
        response.prepare_payload();
    }
};
