//
// cpp14/can_require_concept_not_applicable_free.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "asio/require_concept.hpp"
#include <cassert>

template <int>
struct prop
{
  static constexpr bool is_requirable_concept = true;
};

template <int>
struct object
{
  template <int N>
  friend constexpr object<N> require_concept(const object&, prop<N>)
  {
    return object<N>();
  }
};

int main()
{
  static_assert(!asio::can_require_concept_v<object<1>, prop<2>>, "");
  static_assert(!asio::can_require_concept_v<const object<1>, prop<2>>, "");
}
