//
// Copyright(c) 2016 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once
//
// include bundled or external copy of fmtlib's ostream support
//

#if !defined(SPDLOG_FMT_EXTERNAL)
#ifdef SPDLOG_HEADER_ONLY
#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#endif
#include <spdlog/fmt/bundled/ostream.h>
#else
#include <fmt/ostream.h>
#endif
