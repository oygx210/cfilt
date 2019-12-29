/**
 * Copyright 2020 Feras Boulala <ferasboulala@gmail.com>
 *
 * This file is part of cfilt.
 *
 * cfilt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cfilt is distributed in the hope it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cfilt. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UTEST_H_
#define UTEST_H_

#include "cfilt/util.h"

#include <gsl/gsl_errno.h>

#define UTEST_EXEC_ASSERT(func, ...) EXEC_ASSERT(func, __VA_ARGS__);
#define UTEST_EXEC_ASSERT_(func, ...)                                          \
    do                                                                         \
    {                                                                          \
        const int status_ = func(__VA_ARGS__);                                 \
        if (!status_)                                                          \
        {                                                                      \
            return GSL_EFAILED;                                                \
        }                                                                      \
    } while (0);

#define UTEST_ASSERT(x)                                                        \
    if (x)                                                                     \
        return GSL_EFAILED;
#define UTEST_ASSERT_(x)                                                       \
    if (!x)                                                                    \
        return GSL_EFAILED;

#endif // UTEST_H_
