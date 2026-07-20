#pragma once

#if defined(_MSC_VER)

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <BaseTsd.h>

#ifndef _SSIZE_T_DEFINED
typedef SSIZE_T ssize_t;
#define _SSIZE_T_DEFINED
#endif

// The retained core and the pinned RawSpeed source root use GNU attributes
// for diagnostics and optimization hints. MSVC has no equivalent spelling for
// most of them; removing the hints preserves the program's ABI and behavior.
#ifndef __attribute__
#define __attribute__(...)
#endif

#ifndef __thread
#define __thread __declspec(thread)
#endif

#ifndef __restrict__
#define __restrict__ __restrict
#endif

#ifndef __builtin_unreachable
#define __builtin_unreachable() __assume(0)
#endif

#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#ifdef __cplusplus

#include <limits>
#include <type_traits>

template <typename T>
constexpr bool dt_msvc_add_overflow(T lhs, T rhs, T *result) noexcept
{
  static_assert(std::is_integral_v<T>);

  if constexpr(std::is_signed_v<T>)
  {
    if((rhs > 0 && lhs > std::numeric_limits<T>::max() - rhs)
       || (rhs < 0 && lhs < std::numeric_limits<T>::min() - rhs))
      return true;
  }
  else if(lhs > std::numeric_limits<T>::max() - rhs)
  {
    return true;
  }

  *result = static_cast<T>(lhs + rhs);
  return false;
}

template <typename T>
constexpr bool dt_msvc_mul_overflow(T lhs, T rhs, T *result) noexcept
{
  static_assert(std::is_integral_v<T>);

  if constexpr(std::is_signed_v<T>)
  {
    if((lhs > 0 && ((rhs > 0 && lhs > std::numeric_limits<T>::max() / rhs)
                    || (rhs < 0 && rhs < std::numeric_limits<T>::min() / lhs)))
       || (lhs < 0 && ((rhs > 0 && lhs < std::numeric_limits<T>::min() / rhs)
                    || (rhs < 0 && lhs < std::numeric_limits<T>::max() / rhs))))
      return true;
  }
  else if(rhs != 0 && lhs > std::numeric_limits<T>::max() / rhs)
  {
    return true;
  }

  *result = static_cast<T>(lhs * rhs);
  return false;
}

#ifndef __builtin_sadd_overflow
#define __builtin_sadd_overflow(lhs, rhs, result) dt_msvc_add_overflow((lhs), (rhs), (result))
#endif

#ifndef __builtin_mul_overflow
#define __builtin_mul_overflow(lhs, rhs, result) dt_msvc_mul_overflow((lhs), (rhs), (result))
#endif

#endif
#endif
