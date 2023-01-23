/*
 * tinymt.h (0.1.0-dev)
 *
 * A C++11 header-only implementation of the TinyMT pseudo-random number
 * generator. The latest version and documentation can be found at:
 *
 *   https://github.com/tueda/tinymt-cpp
 *
 * This is based on and heavily influenced by the original code base written by
 * Mutsuo Saito and Makoto Matsumoto, which is available at:
 *
 *   https://github.com/MersenneTwister-Lab/TinyMT
 *
 * =============================================================================
 *
 * Copyright (c) 2011, 2013 Mutsuo Saito, Makoto Matsumoto, Hiroshima University
 * and The University of Tokyo.
 *
 * Copyright (c) 2020 Takahiro Ueda, Seikei University.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of the Hiroshima University nor the names of
 *      its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written
 *      permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TINYMT_TINYMT_H
#define TINYMT_TINYMT_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <limits>
#include <ostream>
#include <type_traits>

/**
 * Macro to enable/disable function via SFINAE.
 *
 * @param ... condition to enable the function
 */
#define TINYMT_CPP_ENABLE_WHEN(...)                                          \
  typename TINYMT_CPP_ENABLE_WHEN_T_ = std::nullptr_t,                       \
           typename std::enable_if < std::is_same<TINYMT_CPP_ENABLE_WHEN_T_, \
                                                  std::nullptr_t>::value &&  \
               (__VA_ARGS__),                                                \
           std::nullptr_t > ::type = nullptr

/**
 * Namespace for classes that implement the TinyMT algorithms.
 */
namespace tinymt {

/**
 * Internal details. They may be incompatibly changed in future. Applications
 * should not rely on these details.
 */
namespace detail {

/**
 * Default parameter `mat1` of TinyMT32 specified in RFC 8682.
 */
constexpr uint_least32_t tinymt32_default_param_mat1 = 0x8f7011eeU;

/**
 * Default parameter `mat2` of TinyMT32 specified in RFC 8682.
 */
constexpr uint_least32_t tinymt32_default_param_mat2 = 0xfc78ff1fU;

/**
 * Default parameter `tmat` of TinyMT32 specified in RFC 8682.
 */
constexpr uint_least32_t tinymt32_default_param_tmat = 0x3793fdffU;

/**
 * Checks whether the given integral type `T` (or its signed type) uses 2's
 * complement for the signed integer representation.
 *
 * @note If `TINYMT_CPP_TWOS_COMPLEMENT` is defined, then its value overwrites
 * the check and totally controls the result.
 */
// This implementation must be probably almost portable; it covers 3 major
// signed integer representations:
//
//                        (-1) & (1)   (-1) & (2)
//   Two's complement              1            2
//   One's complement              0            2
//   Sign and magnitude            1            0
//
// Please point out if there is any compiler/system for which the following code
// gives a wrong result and the compiler still implements the C++11 standard
// correctly (possibly excess-K?).
template <class T>
struct is_twos_complement
    : std::conditional<(typename std::make_signed<T>::type(-1) &
                        typename std::make_signed<T>::type(1)) &&
                           (typename std::make_signed<T>::type(-1) &
                            typename std::make_signed<T>::type(2)),
#ifndef TINYMT_CPP_TWOS_COMPLEMENT
                       std::true_type, std::false_type>::type {
#elif TINYMT_CPP_TWOS_COMPLEMENT
                       std::true_type, std::true_type>::type {
#else
                       std::false_type, std::false_type>::type {
#endif

  static_assert(std::is_integral<T>::value, "T must be an integral type");
};

/**
 * Generator's parameter set.
 */
template <class UIntType>
struct tinymt_engine_param {
  UIntType mat1;
  UIntType mat2;
  UIntType tmat;
};

/**
 * Generator's state and parameter set.
 */
template <class UIntType, std::size_t WordSize, std::uintmax_t Mat1,
          std::uintmax_t Mat2, std::uintmax_t TMat>
struct tinymt_engine_status;

template <class UIntType, std::uintmax_t Mat1, std::uintmax_t Mat2,
          std::uintmax_t TMat>
struct tinymt_engine_status<UIntType, 32, Mat1, Mat2, TMat> {
  using result_type = UIntType;
  std::array<result_type, 4> status;
  static constexpr result_type mat1 = Mat1;
  static constexpr result_type mat2 = Mat2;
  static constexpr result_type tmat = TMat;
  struct is_dynamic : std::false_type {};
};

template <class UIntType>
struct tinymt_engine_status<UIntType, 32, 0, 0, 0>
    : tinymt_engine_param<UIntType> {
  using result_type = UIntType;
  std::array<result_type, 4> status;
  struct is_dynamic : std::true_type {};
};

template <class UIntType1, class UIntType2, std::size_t WordSize,
          std::uintmax_t Mat11, std::uintmax_t Mat12, std::uintmax_t Mat21,
          std::uintmax_t Mat22, std::uintmax_t TMat1, std::uintmax_t TMat2>
inline bool operator==(
    const tinymt_engine_status<UIntType1, WordSize, Mat11, Mat21, TMat1>& a,
    const tinymt_engine_status<UIntType2, WordSize, Mat12, Mat22, TMat2>& b) {
  return a.mat1 == b.mat1 && a.mat2 == b.mat2 && a.tmat == b.tmat &&
         a.status == b.status;
}

template <class UIntType1, class UIntType2, std::size_t WordSize,
          std::uintmax_t Mat11, std::uintmax_t Mat12, std::uintmax_t Mat21,
          std::uintmax_t Mat22, std::uintmax_t TMat1, std::uintmax_t TMat2>
inline bool operator!=(
    const tinymt_engine_status<UIntType1, WordSize, Mat11, Mat21, TMat1>& a,
    const tinymt_engine_status<UIntType2, WordSize, Mat12, Mat22, TMat2>& b) {
  return !(a == b);
}

template <class CharT, class Traits, class UIntType, std::size_t WordSize,
          std::uintmax_t Mat1, std::uintmax_t Mat2, std::uintmax_t TMat>
inline std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,
    const tinymt_engine_status<UIntType, WordSize, Mat1, Mat2, TMat>& s) {
  using status_type =
      tinymt_engine_status<UIntType, WordSize, Mat1, Mat2, TMat>;

  bool first = true;
  for (const auto& x : s.status) {
    if (first) {
      first = false;
    } else {
      os << ' ';
    }
    os << x;
  }

  if (status_type::is_dynamic::value) {
    os << ' ' << s.mat1;
    os << ' ' << s.mat2;
    os << ' ' << s.tmat;
  }

  return os;
}

template <
    class CharT, class Traits, class UIntType, std::size_t WordSize,
    std::uintmax_t Mat1, std::uintmax_t Mat2, std::uintmax_t TMat,
    TINYMT_CPP_ENABLE_WHEN(!tinymt_engine_status<UIntType, WordSize, Mat1, Mat2,
                                                 TMat>::is_dynamic::value)>
inline std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    tinymt_engine_status<UIntType, WordSize, Mat1, Mat2, TMat>& s) {
  for (auto& x : s.status) {
    is >> x >> std::ws;
  }
  return is;
}

template <
    class CharT, class Traits, class UIntType, std::size_t WordSize,
    std::uintmax_t Mat1, std::uintmax_t Mat2, std::uintmax_t TMat,
    TINYMT_CPP_ENABLE_WHEN(tinymt_engine_status<UIntType, WordSize, Mat1, Mat2,
                                                TMat>::is_dynamic::value)>
inline std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    tinymt_engine_status<UIntType, WordSize, Mat1, Mat2, TMat>& s) {
  for (auto& x : s.status) {
    is >> x >> std::ws;
  }
  is >> s.mat1 >> std::ws;
  is >> s.mat2 >> std::ws;
  is >> s.tmat >> std::ws;
  return is;
}

/**
 * Core implementation of the TinyMT algorithms.
 */
template <class UIntType, std::size_t WordSize, std::uintmax_t Mat1,
          std::uintmax_t Mat2, std::uintmax_t TMat, bool DoPeriodCertification>
struct tinymt_engine_impl;

template <class UIntType, std::uintmax_t Mat1, std::uintmax_t Mat2,
          std::uintmax_t TMat, bool DoPeriodCertification>
struct tinymt_engine_impl<UIntType, 32, Mat1, Mat2, TMat,
                          DoPeriodCertification> {
  using result_type = UIntType;
  using signed_result_type = typename std::make_signed<result_type>::type;
  using status_type = tinymt_engine_status<UIntType, 32, Mat1, Mat2, TMat>;
  using param_type = detail::tinymt_engine_param<UIntType>;

  static constexpr std::size_t state_size = 4;
  static constexpr std::uintmax_t max = 0xffffffffU;

  static constexpr std::size_t sh0 = 1;
  static constexpr std::size_t sh1 = 10;
  static constexpr std::size_t sh8 = 8;

  static constexpr result_type word_mask = static_cast<result_type>(max);
  static constexpr result_type mask32 = word_mask;
  static constexpr result_type mask = 0x7fffffffU;

  static void period_certification(status_type& s) {
    // In theory, this may happen but I don't know any example, so it is too
    // difficult to cover these lines.

    // LCOV_EXCL_START
    if ((s.status[0] & mask) == 0 && s.status[1] == 0 && s.status[2] == 0 &&
        s.status[3] == 0) {
      s.status[0] = 'T';
      s.status[1] = 'I';
      s.status[2] = 'N';
      s.status[3] = 'Y';
    }
    // LCOV_EXCL_STOP
  }

  static void init(status_type& s, result_type seed) {
    const unsigned int MIN_LOOP = 8;
    const unsigned int PRE_LOOP = 8;

    // Assume that mat1, mat2, tmat have been suitably initialized.

    s.status[0] = seed & mask32;
    s.status[1] = s.mat1;
    s.status[2] = s.mat2;
    s.status[3] = s.tmat;

    // See Knuth TAOCP Vol2. 3rd Ed. p.106 for the multiplier.

    for (unsigned int i = 1; i < MIN_LOOP; i++) {
      s.status[i & 3] ^= i + 1812433253 * (s.status[(i - 1) & 3] ^
                                           (s.status[(i - 1) & 3] >> 30));
      s.status[i & 3] &= mask32;
    }

    if (DoPeriodCertification) {
      period_certification(s);
    }

    for (unsigned int i = 0; i < PRE_LOOP; i++) {
      next_state(s);
    }
  }

  template <TINYMT_CPP_ENABLE_WHEN(!is_twos_complement<result_type>::value)>
  static void next_state(status_type& s) {
    result_type x = (s.status[0] & mask) ^ s.status[1] ^ s.status[2];
    result_type y = s.status[3];
    x ^= (x << sh0) & mask32;
    y ^= (y >> sh0) ^ x;
    s.status[0] = s.status[1];
    s.status[1] = s.status[2];
    s.status[2] = (x ^ (y << sh1)) & mask32;
    s.status[3] = y;
    if (y & 1) {
      s.status[1] ^= s.mat1;
      s.status[2] ^= s.mat2;
    }
  }

  template <TINYMT_CPP_ENABLE_WHEN(is_twos_complement<result_type>::value)>
  static void next_state(status_type& s) {
    result_type x = (s.status[0] & mask) ^ s.status[1] ^ s.status[2];
    result_type y = s.status[3];
    x ^= (x << sh0) & mask32;
    y ^= (y >> sh0) ^ x;
    s.status[0] = s.status[1];
    s.status[1] = s.status[2];
    s.status[2] = (x ^ (y << sh1)) & mask32;
    s.status[3] = y;
    // NOTE: the conditional branch in the portable version can be removed in
    // the following way using negation in two's complement representation.
    auto ymask =
        static_cast<result_type>(-static_cast<signed_result_type>(y & 1));
    s.status[1] ^= ymask & s.mat1;
    s.status[2] ^= ymask & s.mat2;
  }

  template <TINYMT_CPP_ENABLE_WHEN(!is_twos_complement<result_type>::value)>
  static result_type temper(const status_type& s) {
    result_type t0 = s.status[3];
    result_type t1 = s.status[0] + (s.status[2] >> sh8);
    t0 ^= t1;
    if (t1 & 1) {
      t0 ^= s.tmat;
    }
    return t0 & mask32;
  }

  template <TINYMT_CPP_ENABLE_WHEN(is_twos_complement<result_type>::value)>
  static result_type temper(const status_type& s) {
    result_type t0 = s.status[3];
    result_type t1 = s.status[0] + (s.status[2] >> sh8);
    t0 ^= t1;
    // NOTE: the conditional branch in the portable version can be removed in
    // the following way using negation in two's complement representation.
    auto t1mask =
        static_cast<result_type>(-static_cast<signed_result_type>(t1 & 1));
    t0 ^= t1mask & s.tmat;
    return t0 & mask32;
  }
};

}  // namespace detail

/**
 * Pseudo-random number generator engine based on the TinyMT algorithms.
 *
 * @tparam UIntType unsigned integral type generated by the engine
 * @tparam WordSize word size for the generated numbers
 * @tparam Mat1     parameter used in the linear state transition function
 * @tparam Mat2     parameter used in the linear state transition function
 * @tparam TMat     parameter used in the non-linear output function
 * @tparam DoPeriodCertification whether or not period certification is
 * performed in initialization
 *
 * @note Currently `WordSize` must be `32`.
 * @note When `Mat`, `Mat2` and `TMat` are all zero, the generator parameter set
 * must be provided in initialization ("Dynamic Creation" (DC) mode).
 */
template <class UIntType, std::size_t WordSize, UIntType Mat1, UIntType Mat2,
          UIntType TMat, bool DoPeriodCertification = true>
class tinymt_engine {
  static_assert(std::is_integral<UIntType>::value &&
                    std::is_unsigned<UIntType>::value,
                "result_type must be an unsigned integral type");
  static_assert(WordSize == 32, "word_size must be 32");

  using impl = detail::tinymt_engine_impl<UIntType, WordSize, Mat1, Mat2, TMat,
                                          DoPeriodCertification>;
  using status_type =
      detail::tinymt_engine_status<UIntType, WordSize, Mat1, Mat2, TMat>;

  static_assert(std::numeric_limits<UIntType>::max() >= impl::max,
                "size of result_type must be lager than word_size");
  static_assert(Mat1 <= impl::max, "Mat1 must be < 2^word_size");
  static_assert(Mat2 <= impl::max, "Mat2 must be < 2^word_size");
  static_assert(TMat <= impl::max, "TMat must be < 2^word_size");

  status_type s_;

 public:
  /**
   * Integral type generated by the engine.
   */
  using result_type = UIntType;

  /**
   * Type of the generator parameter set.
   */
  using param_type = typename impl::param_type;

  /**
   * Word size that determines the range of numbers generated by the engine.
   */
  static constexpr std::size_t word_size = WordSize;

  /**
   * Size of the state of the generator in words.
   */
  static constexpr std::size_t state_size = impl::state_size;

  /**
   * Default seed.
   */
  static constexpr result_type default_seed = 1;

  /**
   * Constructs the engine (non-DC mode).
   *
   * @param seed random seed
   */
  template <TINYMT_CPP_ENABLE_WHEN(!status_type::is_dynamic::value)>
  explicit tinymt_engine(result_type seed = default_seed) {
    impl::init(s_, seed);
  }

  /**
   * Constructs the engine (DC mode).
   *
   * @param param parameter set
   * @param seed random seed
   */
  template <TINYMT_CPP_ENABLE_WHEN(status_type::is_dynamic::value)>
  explicit tinymt_engine(const param_type& param,
                         result_type seed = default_seed) {
    s_.mat1 = param.mat1 & impl::word_mask;
    s_.mat2 = param.mat2 & impl::word_mask;
    s_.tmat = param.tmat & impl::word_mask;
    impl::init(s_, seed);
  }

  /**
   * Reinitializes the engine.
   *
   * @param value random seed
   */
  void seed(result_type value = default_seed) { impl::init(s_, value); }

  /**
   * Advances the state of the engine by the given amount.
   *
   * @param z number of advances
   */
  // Note: the use of `unsigned long long` is intentional, following the
  // standard library and the Boost library.
  void discard(unsigned long long z) {            // NOLINT
    for (unsigned long long i = 0; i < z; i++) {  // NOLINT
      impl::next_state(s_);
    }
  }

  /**
   * Returns the smallest possible value in the output range.
   *
   * @return smallest value
   */
  static constexpr result_type min() { return 0; }

  /**
   * Returns the largest possible value in the output range.
   *
   * @return largest value
   */
  static constexpr result_type max() { return impl::max; }

  /**
   * Returns the next pseudo-random number.
   *
   * @return generated value
   */
  result_type operator()() {
    impl::next_state(s_);
    return impl::temper(s_);
  }

  /**
   * Compares two engines.
   *
   * @param a first engine
   * @param b second engine
   * @return `true` if the engines are equivalent including their internal
   * states, `false` otherwise
   */
  friend bool operator==(const tinymt_engine& a, const tinymt_engine& b) {
    return a.s_ == b.s_;
  }

  /**
   * Compares two engines.
   *
   * @param a first engine
   * @param b second engine
   * @return `true` if the engines are not equivalent including their internal
   * states, `false` otherwise
   */
  friend bool operator!=(const tinymt_engine& a, const tinymt_engine& b) {
    return a.s_ != b.s_;
  }

  /**
   * Serializes the state of the given engine into a stream.
   *
   * @param os output stream
   * @param e  engine to be serialized
   * @return `os`
   */
  template <class CharT, class Traits>
  friend std::basic_ostream<CharT, Traits>& operator<<(
      std::basic_ostream<CharT, Traits>& os, const tinymt_engine& e) {
    return os << e.s_;
  }

  /**
   * Deserializes the state of the given engine from a stream.
   *
   * @param is input stream
   * @param e  engine to be deserialized
   * @return `is`
   */
  template <class CharT, class Traits>
  friend std::basic_istream<CharT, Traits>& operator>>(
      std::basic_istream<CharT, Traits>& is, tinymt_engine& e) {
    return is >> e.s_;
  }
};

/**
 * TinyMT32 generator engine with the parameter set specified in RFC 8682.
 */
using tinymt32 =
    tinymt_engine<uint_fast32_t, 32, detail::tinymt32_default_param_mat1,
                  detail::tinymt32_default_param_mat2,
                  detail::tinymt32_default_param_tmat, false>;

/**
 * TinyMT32 generator engine with "Dynamic Creation" of its parameter set.
 */
using tinymt32_dc = tinymt_engine<uint_fast32_t, 32, 0, 0, 0, true>;

}  // namespace tinymt

#endif  // TINYMT_TINYMT_H