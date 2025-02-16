//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BASIC_PARSER_HPP
#define BOOST_HTTP_PROTO_BASIC_PARSER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/buffer.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/detail/header.hpp>
#include <boost/url/grammar/error.hpp>
#include <boost/optional.hpp>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class context;
class request_parser;
class response_parser;
enum class version : char;
#endif

/** A parser for HTTP/1 messages.

    The parser is strict. Any malformed
    inputs according to the documented
    HTTP ABNFs is treated as an
    unrecoverable error.
*/
class BOOST_SYMBOL_VISIBLE
    parser
{
    friend class request_parser;
    friend class response_parser;

public:
    // applies to all messages
    struct config
    {
        /** Largest allowed size for the complete header
        */
        std::size_t max_header_size = 65536;

        /** Largest size for any one field
        */
        std::size_t max_field_size = 4096;

        /** Largest allowed number of fields
        */
        std::size_t max_field_count = 100;

        /** Largest allowed size for a body
        */
        std::uint64_t max_body_size = 1024 * 1024;
    };

private:
    enum class state
    {
        empty,
        start_line,
        header_fields,
        body,
        complete,
    };

    // per-message state
    struct message
    {
        std::size_t n_chunk = 0;    // bytes of chunk header
        std::size_t n_payload = 0;  // bytes of body or chunk
        std::uint64_t n_remain = 0; // remaining body or chunk

        std::uint64_t
            payload_seen = 0;       // total body received

        optional<std::uint64_t>
            content_len;            // value of Content-Length

        bool skip_body : 1;
        bool got_chunked : 1;
        bool got_close : 1;
        bool got_keep_alive : 1;
        bool got_upgrade : 1;
        bool need_eof : 1;

        //message() noexcept;
    };

    config cfg_;
    detail::header h_;

    std::size_t committed_;
    state state_;
    bool got_eof_;
    message m_;
    mutable_buffer mb_;

    parser(
        detail::kind k,
        config const& cfg,
        std::size_t buffer_size);

public:
    using mutable_buffers_type =
        mutable_buffers;

    //--------------------------------------------
    //
    // Special Members
    //
    //--------------------------------------------

    BOOST_HTTP_PROTO_DECL
    ~parser();

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    /** Return true if any input was committed.
    */
    bool
    got_some() const noexcept
    {
        // VFALCO What if discard_header was called?
        return state_ != state::empty;
    }

    /** Return true if the complete header was parsed.
    */
    bool
    got_header() const noexcept
    {
        return state_ > state::header_fields;
    }

    /** Returns true if the payload uses chunked encoding.
    */
    bool
    is_chunked() const noexcept
    {
        return m_.got_chunked;
    }

    /** Returns `true` if a complete message has been parsed.

        Calling @ref reset prepares the parser
        to process the next message in the stream.

    */
    bool
    is_complete() const noexcept
    {
        return state_ ==
            state::complete;
    }

    BOOST_HTTP_PROTO_DECL
    string_view
    body() const noexcept;

    //--------------------------------------------
    //
    // Input
    //
    //--------------------------------------------

    /** Reinitialize the parser for a new stream
    */
    BOOST_HTTP_PROTO_DECL
    void
    clear() noexcept;

    /** Prepare the parser for the next message.
    */
    BOOST_HTTP_PROTO_DECL
    void
    reset();

    /** Return the input buffer
    */
    BOOST_HTTP_PROTO_DECL
    mutable_buffers_type
    prepare();

    /** Commit bytes to the input buffer
    */
    BOOST_HTTP_PROTO_DECL
    void
    commit(
        std::size_t n,
        error_code& ec);

    /** Indicate there will be no more input
    */
    BOOST_HTTP_PROTO_DECL
    void
    commit_eof(
        error_code& ec);

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    BOOST_HTTP_PROTO_DECL
    void
    discard_header() noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    discard_body() noexcept;

    BOOST_HTTP_PROTO_DECL
    void
    discard_chunk() noexcept;

    /** Indicate that the current message has no payload.

        This informs the parser not to read a payload for
        the next message, regardless of the presence or
        absence of certain fields such as Content-Length
        or a chunked Transfer-Encoding. Depending on the
        request, some responses do not carry a body. For
        example, a 200 response to a CONNECT request from
        a tunneling proxy, or a response to a HEAD request.
        In these cases, callers may use this function inform
        the parser that no body is expected. The parser will
        consider the message complete after the header has
        been received.

        @par Preconditions

        This function must called before any calls to parse
        the current message.

        @see
            https://datatracker.ietf.org/doc/html/rfc7230#section-3.3

    */
    BOOST_HTTP_PROTO_DECL
    void
    skip_body();

private:
    void parse(error_code&);
    void parse_start_line(error_code&);
    void parse_fields(error_code&);
    void parse_body(error_code&);
    void parse_chunk(error_code&);

    void do_connection(string_view, error_code&);
    void do_content_length(string_view, error_code&);
    void do_transfer_encoding(string_view, error_code&);
    void do_upgrade(string_view, error_code&);
};

} // http_proto
} // boost

#endif
