#!/bin/sh

g++ -O2 -static -std=c++17 -pthread -DBOOST_COROUTINES_NO_DEPRECATION_WARNING -DBOOST_ASIO_NO_DEPRECATED -DBOOST_ASIO_HEADER_ONLY -DRAPIDJSON_HAS_STDSTRING -o highloadcup2018 main.cpp -lboost_coroutine -lboost_context -lboost_thread -lpthread
