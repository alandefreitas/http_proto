//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_FIELDS_IPP
#define BOOST_HTTP_PROTO_IMPL_FIELDS_IPP

#include <boost/http_proto/fields.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <string>

namespace boost {
namespace http_proto {

fields::
fields() noexcept
    : fields_view_base(
        &this->fields_base::h_)
    , fields_base(
        detail::kind::fields)
{
}

fields::
fields(
    fields&& other) noexcept
    : fields_view_base(
        &this->fields_base::h_)
    , fields_base(other.h_.kind)
{
    swap(other);
}

fields::
fields(
    fields const& other)
    : fields_view_base(
        &this->fields_base::h_)
    , fields_base(*other.ph_)
{
}

fields::
fields(
    fields_view const& other)
    : fields_view_base(
        &this->fields_base::h_)
    , fields_base(*other.ph_)
{
}

fields&
fields::
operator=(
    fields&& other) noexcept
{
    fields tmp(std::move(other));
    tmp.swap(*this);
    return *this;
}

} // http_proto
} // boost

#endif
