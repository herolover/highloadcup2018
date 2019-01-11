#pragma once

#include "../RequestHandler.h"

template<>
struct RequestHandler<RecommendForAccount>
{
    static void handle(DB &db, const RecommendForAccount &request, HttpServer::HttpResponse &response)
    {

    }
};
