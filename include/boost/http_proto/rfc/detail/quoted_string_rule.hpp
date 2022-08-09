//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RULE_QUOTED_STRING_RULE_HPP
#define BOOST_HTTP_PROTO_RULE_QUOTED_STRING_RULE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/url/grammar/parse_tag.hpp>
#include <string>

namespace boost {
namespace http_proto {

/** Rule for quoted-string

    @par BNF
    @code
    quoted-string   = DQUOTE *( qdtext / quoted-pair ) DQUOTE

    qdtext          = HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text
    quoted-pair     = "\" ( HTAB / SP / VCHAR / obs-text )

    obs-text        = %x80-FF
    @endcode

    @par Specification
    @li <a href="https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.6"
        >3.2.6. Field Value Components (rfc7230)</a>
*/
struct quoted_string_rule
{
    using value_type = std::string;
    using reference = string_view;

    reference v;

    reference
    operator*() const noexcept
    {
        return v;
    }

    friend
    void
    tag_invoke(
        grammar::parse_tag const&,
        char const*& it,
        char const* end,
        error_code& ec,
        quoted_string_rule& t) noexcept
    {
        parse(it, end, ec, t);
    }

private:
    BOOST_HTTP_PROTO_DECL
    static
    void
    parse(
        char const*& it,
        char const* end,
        error_code& ec,
        quoted_string_rule& t) noexcept;
};

//------------------------------------------------

BOOST_HTTP_PROTO_DECL
std::string
unquote_text(
    char* start,
    char const* end);

} // http_proto
} // boost

#endif
