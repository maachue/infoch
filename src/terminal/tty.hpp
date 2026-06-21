#ifndef INFOCH_TERMINAL_TTY_H
#define INFOCH_TERMINAL_TTY_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <atomic>
namespace terminal {
using term_t =
#ifdef _WIN32
    HANDLE;
#else
    int;
#endif

#ifdef _WIN32
extern constinit std::atomic<term_t> termin_handle;
extern constinit std::atomic<term_t> termout_handle;
#else
extern constinit std::atomic<term_t> ftty;
#endif
void open_devtty();
void close_devtty();
} // namespace terminal

#endif
