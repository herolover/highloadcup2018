#pragma once

#include "DB.h"
#include "HttpRequestParser.h"
#include "HttpServer.h"

template<class Request>
struct RequestHandler
{
    static void handle(DB &db, Request &request, HttpServer::HttpResponse &response)
    {
        response.result(boost::beast::http::status::not_implemented);
        response.prepare_payload();
    }
};
