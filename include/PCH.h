#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "RE/Starfield.h"
#include "SFSE/SFSE.h"

#include <ankerl/unordered_dense.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <srell.hpp>
#include <tsl/ordered_map.h>
#include <tsl/ordered_set.h>

#include <ClibUtil/distribution.hpp>
#include <ClibUtil/rng.hpp>
#include <ClibUtil/singleton.hpp>
#include <ClibUtil/string.hpp>
#include <ClibUtil/timer.hpp>

#include "LogBuffer.h"

namespace logger = SFSE::log;
namespace buffered_logger = LogBuffer;

namespace string = clib_util::string;
namespace distribution = clib_util::distribution;
namespace hash = clib_util::hash;

using namespace std::literals;
using namespace clib_util::string::literals;

using namespace clib_util::singleton;

using Timer = clib_util::Timer;
using RNG = clib_util::RNG;

namespace stl
{
	using namespace SFSE::stl;
}

#define DLLEXPORT extern "C" [[maybe_unused]] __declspec(dllexport)

#include "Defs.h"
#include "Version.h"
