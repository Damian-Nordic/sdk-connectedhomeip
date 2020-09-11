/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      Utilities for safely working with integer types.
 *
 */

#ifndef SAFEINT_H_
#define SAFEINT_H_

#include <limits>
#include <type_traits>

namespace chip {

/**
 * A template function that determines whether it's safe to cast the given value
 * of type U to the given type T.  It does this by verifying that the value is
 * in the range of valid values for T.
 */
template <typename T, typename U>
bool CanCastTo(U arg)
{
    using namespace std;
    // U might be a reference to an integer type, if we're assigning from
    // something passed by reference.
    typedef typename remove_reference<U>::type V; // V for "value"
    static_assert(is_integral<T>::value, "Must be assigning to an integral type");
    static_assert(is_integral<V>::value, "Must be assigning from an integral type");

    // We want to check that "arg" can fit inside T but without doing any tests
    // that are always true or always false due to the types involved, which
    // would trigger compiler warnings themselves.  So for example, we can't
    // can't compare arg to max values for T if all U values are representable
    // in T, etc, because those trigger warnings on some compilers.

    // We also can't directly compare signed to unsigned values in general,
    // because that will trigger sign conversion warnings. In fact, it will
    // trigger them even on runtime-unreached codepaths, so for example we can't
    // directly compare two min() values to each other!

    // Oh, and some compilers warn on theoretical signed-to-unsigned compares
    // even when those can't be reached, and that's known at compile time.
    // Hence all the casts to intmax_t and uintmax_t below.

    // A bunch of these tests could sure benefit from "if constexpr", but let's
    // hope compilers just manage to optimize them properly anyway.
    // We can't blindly compare "arg" to the minimal or maximal value of T one
    // of T and V is signed and the other is unsigned: there might not be a
    // single integer type that can represent _both_ the value of arg and the
    // minimal/maximal value.
    if (numeric_limits<T>::is_signed && numeric_limits<V>::is_signed)
    {
        if (static_cast<intmax_t>(numeric_limits<V>::max()) <= static_cast<intmax_t>(numeric_limits<T>::max()) &&
            static_cast<intmax_t>(numeric_limits<V>::min()) >= static_cast<intmax_t>(numeric_limits<T>::min()))
        {
            // Any checks on arg would be trivially true; don't even do them, to
            // avoid warnings.
            return true;
        }

        return static_cast<intmax_t>(numeric_limits<T>::min()) <= static_cast<intmax_t>(arg) &&
            static_cast<intmax_t>(arg) <= static_cast<intmax_t>(numeric_limits<T>::max());
    }

    if (!numeric_limits<T>::is_signed && !numeric_limits<V>::is_signed)
    {
        if (static_cast<uintmax_t>(numeric_limits<V>::max()) <= static_cast<uintmax_t>(numeric_limits<T>::max()))
        {
            // Any checks on arg would be trivially true; don't even do them, to
            // avoid warnings.
            return true;
        }

        return static_cast<uintmax_t>(arg) <= static_cast<uintmax_t>(numeric_limits<T>::max());
    }

    if (numeric_limits<T>::is_signed)
    {
        static_assert(numeric_limits<T>::max() >= 0, "What weird type is this?");
        if (static_cast<uintmax_t>(numeric_limits<V>::max()) <= static_cast<uintmax_t>(numeric_limits<T>::max()))
        {
            return true;
        }

        return static_cast<uintmax_t>(arg) <= static_cast<uintmax_t>(numeric_limits<T>::max());
    }

    return 0 <= arg && static_cast<uintmax_t>(arg) <= static_cast<uintmax_t>(numeric_limits<T>::max());
}

} // namespace chip

#endif // SAFEINT_H_