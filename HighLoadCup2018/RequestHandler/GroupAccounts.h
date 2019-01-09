#pragma once

#include "../RequestHandler.h"

template<>
struct RequestHandler<GroupAccounts>
{
    static void handle(DB &db, const FilterAccounts &request, HttpServer::HttpResponse &response)
    {
    }
};
