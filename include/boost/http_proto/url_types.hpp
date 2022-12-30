//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_URL_TYPES_HPP
#define BOOST_HTTP_PROTO_URL_TYPES_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/url/authority_view.hpp>
#include <boost/url/url_view.hpp>

namespace boost {
namespace http_proto {

using url_view = urls::url_view;
using authority_view = urls::authority_view;

} // http_proto
} // boost

#endif
