
#ifndef _ZEQ_DEFINE_HPP_
#define _ZEQ_DEFINE_HPP_

#include "perf_timer.hpp"
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <cctype>
#include <cmath>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#ifdef _WIN32
#include <windows.h>
#define ZEQ_EXPORT extern "C" __declspec(dllexport)
#define ZEQ_WINDOWS
#else
#include <cerrno>
#include <cinttypes>
#define ZEQ_EXPORT extern "C"
#endif

typedef uint8_t byte;

#endif//_ZEQ_DEFINE_HPP_
