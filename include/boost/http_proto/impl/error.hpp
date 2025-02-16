//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_ERROR_HPP
#define BOOST_HTTP_PROTO_IMPL_ERROR_HPP

namespace boost {
namespace http_proto {

BOOST_HTTP_PROTO_DECL
error_code
make_error_code(
    error ev) noexcept;

} // http_proto
} // boost

#endif
