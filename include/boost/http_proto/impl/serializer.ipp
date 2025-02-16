//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_SERIALIZER_IPP
#define BOOST_HTTP_PROTO_IMPL_SERIALIZER_IPP

#include <boost/http_proto/serializer.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/core/ignore_unused.hpp>
#include <stddef.h>

namespace boost {
namespace http_proto {

//------------------------------------------------

void
consume_buffers(
    const_buffer*& p,
    std::size_t& n,
    std::size_t bytes)
{
    while(n > 0)
    {
        if(bytes < p->size())
        {
            *p += bytes;
            return;
        }
        bytes -= p->size();
        ++p;
        --n;
    }

    // Precondition violation
    if(bytes > 0)
        detail::throw_invalid_argument();
}

template<class MutableBuffers>
void
write_chunk_header(
    MutableBuffers const& dest0,
    std::size_t size) noexcept
{
    static constexpr char hexdig[] =
        "0123456789ABCDEF";
    char buf[18];
    auto p = buf + 16;
    for(std::size_t i = 16; i--;)
    {
        *--p = hexdig[size & 0xf];
        size >>= 4;
    }
    buf[16] = '\r';
    buf[17] = '\n';
    auto n = buffer_copy(
        (make_buffers)(dest0),
        const_buffer(buf, sizeof(buf)));
    ignore_unused(n);
    BOOST_ASSERT(n == 18);
    BOOST_ASSERT(
        buffer_size(dest0) == n);
}

//------------------------------------------------

class serializer::
    reserve
    : public source::reserve_fn
{
    serializer& sr_;
    std::size_t limit_;

public:
    reserve(
        serializer& sr,
        std::size_t limit) noexcept
        : sr_(sr)
        , limit_(limit)
    {
    }

    void*
    operator()(
        std::size_t n) const override
    {
        // You can only call reserve() once!
        if(! sr_.is_reserving_ )
            detail::throw_logic_error();

        // Requested size exceeds the limit
        if(n > limit_)
            detail::throw_length_error();

        sr_.is_reserving_ = false;
        return sr_.ws_.reserve(n);
    }
};

//------------------------------------------------

serializer::
~serializer()
{
}

serializer::
serializer()
    : serializer(65536)
{
}

serializer::
serializer(
    std::size_t buffer_size)
    : ws_(buffer_size)
{
}

//------------------------------------------------

auto
serializer::
prepare() ->
    result<output>
{
    // Precondition violation
    if(is_done_)
        detail::throw_logic_error();

    // Expect: 100-continue
    if(is_expect_continue_)
    {
        if(out_.data() == hp_)
            return output(hp_, 1);
        is_expect_continue_ = false;
        BOOST_HTTP_PROTO_RETURN_EC(
            error::expect_100_continue);
    }

    if(st_ == style::empty)
    {
        return output(
            out_.data(),
            out_.size());
    }

    if(st_ == style::buffers)
    {
        return output(
            out_.data(),
            out_.size());
    }

    if(st_ == style::source)
    {
        if(! is_chunked_)
        {
            auto rv = src_->read(
                tmp0_.prepare(
                    tmp0_.capacity() -
                        tmp0_.size()));
            tmp0_.commit(rv.bytes);
            if(rv.ec.failed())
                return rv.ec;
            more_ = rv.more;
        }
        else
        {
            if((tmp0_.capacity() -
                    tmp0_.size()) >
                chunked_overhead_)
            {
                auto dest = tmp0_.prepare(18);
                write_chunk_header(dest, 0);
                tmp0_.commit(18);
                auto rv = src_->read(
                    tmp0_.prepare(
                        tmp0_.capacity() -
                            2 - // CRLF
                            5 - // final chunk
                            tmp0_.size()));
                tmp0_.commit(rv.bytes);
                if(rv.bytes == 0)
                    tmp0_.uncommit(18); // undo
                if(rv.ec.failed())
                    return rv.ec;
                if(rv.bytes > 0)
                {
                    // rewrite with correct size
                    write_chunk_header(
                        dest, rv.bytes);
                    // terminate chunk
                    tmp0_.commit(buffer_copy(
                        tmp0_.prepare(2),
                        const_buffer(
                            "\r\n", 2)));
                }
                if(! rv.more)
                {
                    tmp0_.commit(buffer_copy(
                        tmp0_.prepare(5),
                        const_buffer(
                            "0\r\n\r\n", 5)));
                }
                more_ = rv.more;
            }
        }

        std::size_t n = 0;
        if(out_.data() == hp_)
            ++n;
        for(const_buffer const& b : tmp0_.data())
            out_[n++] = b;

        return output(
            out_.data(),
            out_.size());
    }

    if(st_ == style::stream)
    {
        std::size_t n = 0;
        if(out_.data() == hp_)
            ++n;
        if(tmp0_.empty() && more_)
        {
            BOOST_HTTP_PROTO_RETURN_EC(
                error::need_data);
        }
        for(const_buffer const& b : tmp0_.data())
            out_[n++] = b;

        return output(
            out_.data(),
            out_.size());
    }

    // should never get here
    detail::throw_logic_error();
}

void
serializer::
consume(
    std::size_t n)
{
    // Precondition violation
    if(is_done_)
        detail::throw_logic_error();

    if(is_expect_continue_)
    {
        // Cannot consume more than
        // the header on 100-continue
        if(n > hp_->size())
            detail::throw_invalid_argument();

        out_.consume(n);
        return;
    }
    else if(out_.data() == hp_)
    {
        // consume header
        if(n < hp_->size())
        {
            out_.consume(n);
            return;
        }
        n -= hp_->size();
        out_.consume(hp_->size());
    }

    switch(st_)
    {
    default:
    case style::empty:
        out_.consume(n);
        if(out_.empty())
            is_done_ = true;
        return;

    case style::buffers:
        out_.consume(n);
        if(out_.empty())
            is_done_ = true;
        return;

    case style::source:
    case style::stream:
        tmp0_.consume(n);
        if( tmp0_.empty() &&
                ! more_)
            is_done_ = true;
        return;
    }
}

//------------------------------------------------

void
serializer::
copy(
    const_buffer* dest,
    const_buffer const* src,
    std::size_t n) noexcept
{
    while(n--)
        *dest++ = *src++;
}

void
serializer::
do_maybe_reserve(
    source& src,
    std::size_t limit)
{
    struct cleanup
    {
        bool& is_reserving;

        ~cleanup()
        {
            is_reserving = false;
        }
    };

    BOOST_ASSERT(! is_reserving_);
    cleanup c{is_reserving_};
    is_reserving_ = true;
    reserve fn(*this, limit);
    src.maybe_reserve(limit, fn);
}

void
serializer::
reset_init(
    message_view_base const& m)
{
    ws_.clear();

    // VFALCO what do we do with
    // metadata error code failures?
    // m.ph_->md.maybe_throw();

    is_done_ = false;

    is_expect_continue_ =
        m.ph_->md.expect.is_100_continue;

    // Transfer-Encoding
    {
        auto const& te =
            m.ph_->md.transfer_encoding;
        is_chunked_ = te.is_chunked;
        cod_ = nullptr;
    }
}

void
serializer::
reset_empty_impl(
    message_view_base const& m)
{
    reset_init(m);

    st_ = style::empty;

    if(! is_chunked_)
    {
        out_ = make_array(
            1); // header
    }
    else
    {
        out_ = make_array(
            1 + // header
            1); // final chunk

        // Buffer is too small
        if(ws_.size() < 5)
            detail::throw_length_error();

        mutable_buffer dest(
            ws_.data(), 5);
        buffer_copy(
            dest,
            const_buffer(
                "0\r\n\r\n", 5));
        out_[1] = dest;
    }

    hp_ = &out_[0];
    *hp_ = { m.ph_->cbuf, m.ph_->size };
}

void
serializer::
reset_buffers_impl(
    message_view_base const& m)
{
    st_ = style::buffers;

    if(! is_chunked_)
    {
        if(! cod_)
        {
            out_ = make_array(
                1 +             // header
                buf_.size());   // body
            copy(&out_[1],
                buf_.data(), buf_.size());
        }
        else
        {
            out_ = make_array(
                1 + // header
                2); // tmp1
        }
    }
    else
    {
        if(! cod_)
        {
            out_ = make_array(
                1 +             // header
                1 +             // chunk size
                buf_.size() +   // body
                1);             // final chunk
            copy(&out_[2],
                buf_.data(), buf_.size());

            // Buffer is too small
            if(ws_.size() < 18 + 7)
                detail::throw_length_error();
            mutable_buffer s1(ws_.data(), 18);
            mutable_buffer s2(ws_.data(), 18 + 7);
            s2 += 18; // VFALCO HACK
            write_chunk_header(
                s1,
                buffer_size(buf_));
            buffer_copy(s2, const_buffer(
                "\r\n"
                "0\r\n"
                "\r\n", 7));
            out_[1] = s1;
            out_[out_.size() - 1] = s2;
        }
        else
        {
            out_ = make_array(
                1 +     // header
                2);     // tmp1
        }
    }

    hp_ = &out_[0];
    *hp_ = { m.ph_->cbuf, m.ph_->size };
}

void
serializer::
reset_source_impl(
    message_view_base const& m,
    source* src)
{
    st_ = style::source;
    src_ = src;
    do_maybe_reserve(
        *src, ws_.size() / 2);
    out_ = make_array(
        1 + // header
        2); // tmp
    if(! cod_)
    {
        tmp0_ = { ws_.data(), ws_.size() };
        if(tmp0_.capacity() <
                18 +    // chunk size
                1 +     // body (1 byte)
                2 +     // CRLF
                5)      // final chunk
            detail::throw_length_error();
    }
    else
    {
        auto const n = ws_.size() / 2;

        // Buffer is too small
        if(n < 1)
            detail::throw_length_error();

        tmp0_ = { ws_.reserve(n), n };

        // Buffer is too small
        if(ws_.size() < 1)
            detail::throw_length_error();

        tmp1_ = { ws_.data(), ws_.size() };
    }

    hp_ = &out_[0];
    *hp_ = { m.ph_->cbuf, m.ph_->size };
}

void
serializer::
reset_stream_impl(
    message_view_base const& m,
    source& src)
{
    reset_init(m);

    st_ = style::stream;
    do_maybe_reserve(
        src, ws_.size() / 2);
    out_ = make_array(
        1 + // header
        2); // tmp
    if(! cod_)
    {
        tmp0_ = { ws_.data(), ws_.size() };
        if(tmp0_.capacity() <
                18 +    // chunk size
                1 +     // body (1 byte)
                2 +     // CRLF
                5)      // final chunk
            detail::throw_length_error();
    }
    else
    {
        auto const n = ws_.size() / 2;

        // Buffer is too small
        if(n < 1)
            detail::throw_length_error();

        tmp0_ = { ws_.reserve(n), n };

        // Buffer is too small
        if(ws_.size() < 1)
            detail::throw_length_error();

        tmp1_ = { ws_.data(), ws_.size() };
    }

    hp_ = &out_[0];
    *hp_ = { m.ph_->cbuf, m.ph_->size };

    more_ = true;
}

//------------------------------------------------

std::size_t
serializer::
stream::
capacity() const
{
    auto const n = 
        chunked_overhead_ +
            2 + // CRLF
            5;  // final chunk
    return sr_->tmp0_.capacity() - n; // VFALCO ?
}

std::size_t
serializer::
stream::
size() const
{
    return sr_->tmp0_.size();
}

auto
serializer::
stream::
prepare(
    std::size_t n) const ->
        buffers_type
{
    return sr_->tmp0_.prepare(n);
}

void
serializer::
stream::
commit(std::size_t n) const
{
    sr_->tmp0_.commit(n);
}

void
serializer::
stream::
close() const
{
    // Precondition violation
    if(! sr_->more_)
        detail::throw_logic_error();
    sr_->more_ = false;
}

//------------------------------------------------

} // http_proto
} // boost

#endif
