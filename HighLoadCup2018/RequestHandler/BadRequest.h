#pragma once

#include "../RequestHandler.h"

template<>
struct RequestHandler<BadRequest>
{
    static void handle(DB &db, const BadRequest &request, HttpServer::HttpResponse &response)
    {
        response.result(request.status);
        response.prepare_payload();
    }
};
