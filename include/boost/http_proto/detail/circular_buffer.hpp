//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_CIRCULAR_BUFFER_HPP
#define BOOST_HTTP_PROTO_DETAIL_CIRCULAR_BUFFER_HPP

#include <boost/http_proto/buffer.hpp>

namespace boost {
namespace http_proto {
namespace detail {

class circular_buffer
{
    unsigned char* base_ = nullptr;
    std::size_t cap_ = 0;
    std::size_t in_pos_ = 0;
    std::size_t in_len_ = 0;
    std::size_t out_size_ = 0;

public:
    circular_buffer() = default;
    circular_buffer(
        circular_buffer const&) = default;
    circular_buffer& operator=(
        circular_buffer const&) = default;

    circular_buffer(
        void* base,
        std::size_t capacity) noexcept;

    bool empty() const noexcept;
    std::size_t size() const noexcept;
    std::size_t capacity() const noexcept;
    const_buffers_pair data() const noexcept;
    mutable_buffers_pair prepare(std::size_t n);
    void commit(std::size_t n);
    void uncommit(std::size_t n);
    void consume(std::size_t n) noexcept;
};

} // detail
} // http_proto
} // boost

#endif
