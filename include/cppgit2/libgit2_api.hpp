#pragma once
#include <cppgit2/git_exception.hpp>
#include <git2.h>
#include <iostream>
#include <tuple>

namespace cppgit2 {

class libgit2_api {
public:
  libgit2_api() { git_libgit2_init(); }

  ~libgit2_api() { git_libgit2_shutdown(); }

  std::tuple<int, int, int> version() const {
    int major, minor, revision;
    git_exception::throw_nonzero(
        git_libgit2_version(&major, &minor, &revision));
    return std::tuple<int, int, int>{major, minor, revision};
  }
};

} // namespace cppgit2
