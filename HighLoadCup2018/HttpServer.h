#pragma once

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include <iostream>

class HttpServer
{
public:
    using HttpRequest = boost::beast::http::request<boost::beast::http::string_body>;
    using HttpResponse = boost::beast::http::response<boost::beast::http::string_body>;

    HttpServer(boost::asio::io_context &io_context, uint16_t port)
        : _io_context(io_context)
        , _acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::any(), port), true)
    {
    }

    template<class Handler>
    void start(Handler &&handler)
    {
        boost::asio::spawn(_io_context, [this, handler = std::forward<Handler>(handler)](boost::asio::yield_context yield) mutable
        {
            try
            {
                while (true)
                {
                    boost::asio::ip::tcp::socket socket(_io_context);
                    _acceptor.async_accept(socket, yield);

                    boost::asio::spawn(_io_context, [socket = std::move(socket), handler = std::forward<Handler>(handler)](boost::asio::yield_context yield) mutable
                    {
                        boost::system::error_code ec;

                        boost::beast::flat_buffer buffer;
                        while (true)
                        {
                            HttpRequest request;
                            boost::beast::http::async_read(socket, buffer, request, yield[ec]);

                            if (ec)
                            {
                                break;
                            }

                            auto need_eof = request.need_eof();

                            HttpResponse response;
                            if (!need_eof)
                            {
                                response.set(boost::beast::http::field::connection, "keep-alive");
                            }
                            response.set(boost::beast::http::field::content_type, "application/json");
                            handler(request, response);
                            boost::beast::http::async_write(socket, response, yield[ec]);

                            if (ec)
                            {
                                socket.close(ec);
                                break;
                            }
                            else if (need_eof)
                            {
                                socket.shutdown(boost::asio::socket_base::shutdown_both, ec);
                                socket.close(ec);
                                break;
                            }
                        }
                    });
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << std::endl;
            }
        });
    }

private:
    boost::asio::io_context &_io_context;
    boost::asio::ip::tcp::acceptor _acceptor;
};
