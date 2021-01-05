//
// Copyright(c) 2016-2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

//
// Include a bundled header-only copy of fmtlib or an external one.
// By default spdlog include its own copy.
//

#if !defined(SPDLOG_FMT_EXTERNAL)
#if !defined(SPDLOG_COMPILED_LIB) && !defined(FMT_HEADER_ONLY)
#define FMT_HEADER_ONLY
#endif
#ifndef FMT_USE_WINDOWS_H
#define FMT_USE_WINDOWS_H 0
#endif
// enable the 'n' flag in for backward compatibility with fmt 6.x
#define FMT_DEPRECATED_N_SPECIFIER
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>
#else // SPDLOG_FMT_EXTERNAL is defined - use external fmtlib
#include <fmt/core.h>
#include <fmt/format.h>
#endif