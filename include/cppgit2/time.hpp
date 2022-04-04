#pragma once
#include <cppgit2/libgit2_api.hpp>
#include <git2.h>

namespace cppgit2 {

using epoch_time_seconds = git_time_t;
using offset_minutes = int;

} // namespace cppgit2
