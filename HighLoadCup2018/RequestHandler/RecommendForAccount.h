#pragma once

#include "../RequestHandler.h"

template<>
struct RequestHandler<RecommendForAccount>
{
    static void handle(DB &db, RecommendForAccount &request, HttpServer::HttpResponse &response)
    {

    }
};
