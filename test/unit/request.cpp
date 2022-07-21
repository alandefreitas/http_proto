//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/request.hpp>

#include "test_helpers.hpp"

#include <utility>

namespace boost {
namespace http_proto {

class request_test
{
public:
#if 0
    void
    testHelpers()
    {
        string_view const cs =
            "POST /x HTTP/1.0\r\n"
            "User-Agent: boost\r\n"
            "\r\n";
        request_view rv = make_request(cs);
        request req(rv);
        BOOST_TEST(req.method() == method::post);
        BOOST_TEST(req.method_str() == "POST");
        BOOST_TEST(req.target() == "/x");
        BOOST_TEST(req.version() == version::http_1_0);
        BOOST_TEST(req.string().data() != cs.data());
        BOOST_TEST(req.string() == cs);
    }

    void
    testSpecial()
    {
        string_view const cs =
            "POST /x HTTP/1.0\r\n"
            "Content-Length: 42\r\n"
            "User-Agent: boost\r\n"
            "\r\n";

        string_view const cs2 =
            "CONNECT 127.0.0.1 HTTP/1.1\r\n"
            "User-Agent: boost\r\n"
            "\r\n";

        // request()
        {
            request req;
            check(req, 0,
                "GET / HTTP/1.1\r\n"
                "\r\n");
            BOOST_TEST(
                req.method() == method::get);
            BOOST_TEST(
                req.method_str() == "GET");
            BOOST_TEST(
                req.target() == "/");
            BOOST_TEST(
                req.version() == version::http_1_1);
        }

        // request(request const&)
        {
            {
                // default
                request r1;
                request r2(r1);
                check(r2, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
                BOOST_TEST(
                    r2.method() == method::get);
                BOOST_TEST(
                    r2.method_str() == "GET");
                BOOST_TEST(
                    r2.version() == version::http_1_1);
            }
            {
                request r1 =
                    make_request(cs);
                request r2(r1);
                check(r1, 2, cs);
                check(r2, 2, cs);
                BOOST_TEST(
                    r2.string().data() !=
                        r1.string().data());
                BOOST_TEST(
                    r2.method() == method::post);
                BOOST_TEST(
                    r2.method_str() == "POST");
                BOOST_TEST(
                    r2.version() == version::http_1_0);
            }
        }

        // request(request&&)
        {
            {
                // default
                request r1;
                request r2(std::move(r1));
                check(r1, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
                check(r2, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
                BOOST_TEST(
                    r2.method() == method::get);
                BOOST_TEST(
                    r2.method_str() == "GET");
                BOOST_TEST(
                    r2.version() == version::http_1_1);
            }
            {
                request r1 =
                    make_request(cs);
                request r2(std::move(r1));
                check(r1, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
                check(r2, 2, cs);
                BOOST_TEST(
                    r2.string().data() !=
                        r1.string().data());
                BOOST_TEST(
                    r2.method() == method::post);
                BOOST_TEST(
                    r2.method_str() == "POST");
                BOOST_TEST(
                    r2.version() == version::http_1_0);
            }
        }

        // operator=(request const&)
        {
            {
                // default
                request r1;
                request r2 = make_request(cs);
                r1 = r2;
                check(r1, 2, cs);
                check(r2, 2, cs);
                BOOST_TEST(
                    r2.string().data() !=
                        r1.string().data());
                BOOST_TEST(
                    r1.method() == method::post);
                BOOST_TEST(
                    r1.method_str() == "POST");
                BOOST_TEST(
                    r1.version() == version::http_1_0);
            }
            {
                request r1 = make_request(cs);
                request r2;
                r1 = r2;
                check(r1, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
                BOOST_TEST(
                    r1.string().data() ==
                        r2.string().data());
                BOOST_TEST(
                    r1.method() == method::get);
                BOOST_TEST(
                    r1.method_str() == "GET");
                BOOST_TEST(
                    r1.version() == version::http_1_1);
            }
        }

        // operator=(fields&&)
        {
            {
                request r1 = make_request(cs);
                request r2;
                r2 = std::move(r1);
                check(r1, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
                check(r2, 2, cs);
                BOOST_TEST(
                    r2.method() == method::post);
                BOOST_TEST(
                    r2.method_str() == "POST");
                BOOST_TEST(
                    r2.target() == "/x");
                BOOST_TEST(
                    r2.version() == version::http_1_0);
            }
            {
                request r1 = make_request(cs);
                request r2 = make_request(cs2);
                r2 = std::move(r1);
                check(r1, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
                check(r2, 2, cs);
                BOOST_TEST(
                    r2.method() == method::post);
                BOOST_TEST(
                    r2.method_str() == "POST");
                BOOST_TEST(
                    r2.target() == "/x");
                BOOST_TEST(
                    r2.version() == version::http_1_0);
            }
            {
                request r1;
                request r2 = make_request(cs);
                r2 = std::move(r1);
                check(r1, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
                check(r2, 0,
                    "GET / HTTP/1.1\r\n"
                    "\r\n");
                BOOST_TEST(
                    r1.string().data() ==
                    r2.string().data());
                BOOST_TEST(
                    r2.method() == method::get);
                BOOST_TEST(
                    r2.method_str() == "GET");
                BOOST_TEST(
                    r2.version() == version::http_1_1);
            }
        }
    }

    void
    testObservers()
    {
    }

    void
    testModifiers()
    {
        // clear()
        {
            {
                request req;
                BOOST_TEST(req.capacity_in_bytes() == 0);
                req.clear();
                BOOST_TEST(req.capacity_in_bytes() == 0);
                BOOST_TEST(req.string() ==
                    "GET / HTTP/1.1\r\n\r\n");
            }
            {
                request req = make_request(
                    "POST /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                auto const n =
                    req.capacity_in_bytes();
                BOOST_TEST(n > 0);
                req.clear();
                BOOST_TEST(req.size() == 0);
                BOOST_TEST(
                    req.capacity_in_bytes() == n);
                BOOST_TEST(
                    req.method() == method::get);
                BOOST_TEST(
                    req.method_str() == "GET");
                BOOST_TEST(
                    req.target() == "/");
                BOOST_TEST(
                    req.version() == version::http_1_1);
                BOOST_TEST(req.string() ==
                    "GET / HTTP/1.1\r\n\r\n");
            }
        }

        // set_method(method)
        {
            {
                request req;
                req.set_method(method::delete_);
                BOOST_TEST(
                    req.method() == method::delete_);
                BOOST_TEST(
                    req.method_str() == "DELETE");
                BOOST_TEST(req.string() ==
                    "DELETE / HTTP/1.1\r\n\r\n");
            }
            {
                request req = make_request(
                    "POST /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_method(method::delete_);
                BOOST_TEST(
                    req.method() == method::delete_);
                BOOST_TEST(
                    req.method_str() == "DELETE");
                BOOST_TEST(req.string() ==
                    "DELETE /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
        }

        // set_method(string_view)
        {
            {
                request req;
                req.set_method("DELETE");
                BOOST_TEST(
                    req.method() == method::delete_);
                BOOST_TEST(
                    req.method_str() == "DELETE");
                BOOST_TEST(req.string() ==
                    "DELETE / HTTP/1.1\r\n\r\n");
            }
            {
                request req = make_request(
                    "POST /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_method("DELETE");
                BOOST_TEST(
                    req.method() == method::delete_);
                BOOST_TEST(
                    req.method_str() == "DELETE");
                BOOST_TEST(req.string() ==
                    "DELETE /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
            {
                request req = make_request(
                    "BOOST /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_method("BOOST");
                BOOST_TEST(
                    req.method() == method::unknown);
                BOOST_TEST(
                    req.method_str() == "BOOST");
                BOOST_TEST(req.string() ==
                    "BOOST /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
        }

        // set_target
        {
            {
                request req;
                req.set_target("/index.htm");
                BOOST_TEST(
                    req.target() == "/index.htm");
                BOOST_TEST(req.string() ==
                    "GET /index.htm HTTP/1.1\r\n\r\n");
            }
            {
                request req = make_request(
                    "POST /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_target("/index.htm");
                BOOST_TEST(req.string() ==
                    "POST /index.htm HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
        }

        // set_version
        {
            {
                request req;
                req.set_version(version::http_1_0);
                BOOST_TEST(
                    req.version() == version::http_1_0);
                BOOST_TEST(req.string() ==
                    "GET / HTTP/1.0\r\n\r\n");
            }
            {
                request req = make_request(
                    "POST /x HTTP/1.1\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
                req.set_version(version::http_1_0);
                BOOST_TEST(
                    req.version() == version::http_1_0);
                BOOST_TEST(req.string() ==
                    "POST /x HTTP/1.0\r\n"
                    "User-Agent: boost\r\n"
                    "\r\n");
            }
        }
    }
#endif

    void
    testContentLength()
    {
        auto const check = [](
            request const& req,
            content_length cl1)
        {
            auto const cl0 =
                req.content_length();
            BOOST_TEST_EQ(
                cl0.count, cl1.count);
            BOOST_TEST_EQ(
                cl0.value, cl1.value);
            BOOST_TEST_EQ(
                cl0.has_value, cl1.has_value);
        };

        request req;
        check(req, { 0, 0, false });

        req.append(field::content_length, "0");
        check(req, { 1, 0, true });

        req.set(field::content_length, "1");
        check(req, { 1, 1, true });

        req.append(field::content_length, "1");
        check(req, { 2, 1, true });

        req.erase(field::content_length);
        check(req, { 0, 0, false });

        req.set(field::content_length, "2");
        check(req, { 1, 2, true });

        // non-matching values
        req.append(field::content_length, "3");
        check(req, { 2, 0, false });

        // back to one value
        req.erase(req.find(field::content_length));
        check(req, { 1, 3, true });

        req.set_content_length(42);
        check(req, { 1, 42, true });

        req.set_content_length(0);
        check(req, { 1, 0, true });

        // overflow
        req.set(field::content_length,
            "18446744073709551616");
        check(req, { 1, 0, false });
        req.append(field::content_length, "42");
        check(req, { 2, 0, false });

        // back to one value
        req.erase(req.find(field::content_length));
        check(req, { 1, 42, true });
    }

    void
    run()
    {
#if 0
        testHelpers();
        testSpecial();
        testObservers();
        testModifiers();
#endif
        testContentLength();
    }
};

TEST_SUITE(
    request_test,
    "boost.http_proto.request");

} // http_proto
} // boost
