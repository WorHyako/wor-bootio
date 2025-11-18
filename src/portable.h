#pragma once

#if __STDC_VERSION__ == 201710L

#define wor_bootio_nodiscard__
#define wor_bootio_constexpr__ const
#define wor_bootio_nullptr__ NULL

#elif __STDC_VERSION__ == 202311L

#define wor_bootio_nodiscard__ [[nodiscard]]
#define wor_bootio_constexpr__ constexpr
#define wor_bootio_nullptr__ nullptr

#elif
static_assert(false, "Only MSVC (C17) and LLVM Clang (C17/C23) compilers are supported.")
#endif

#if defined(_MSC_VER)
#include <Windows.h>
#elif defined(__clang__)
#include <time.h>
#include <unistd.h>
#endif

/**
 * \brief
 *
 * \param milliseconds
 */
static void wor_bootio_sleep_ms(const int milliseconds) {
#ifdef WIN32
    Sleep(milliseconds);
#else
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, wor_bootio_nullptr__);
#endif
}
