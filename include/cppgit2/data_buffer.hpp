#pragma once
#include <cppgit2/git_exception.hpp>
#include <cppgit2/libgit2_api.hpp>
#include <cstdlib>
#include <cstring>
#include <git2.h>
#include <string>

namespace cppgit2 {

class data_buffer : public libgit2_api {
public:
  // Default construct a data buffer using GIT_BUF_INIT
  data_buffer();

  // Construct buffer from libgit2 C ptr
  data_buffer(const git_buf *c_ptr);

  // Dispose internal buffer
  ~data_buffer();

  // Move constructor (appropriate other's c_struct_)
  data_buffer(data_buffer&& other);

  // Move assignment constructor (appropriate other's c_struct_)
  data_buffer& operator= (data_buffer&& other);

  // Check quickly if buffer contains a NUL byte
  bool contains_nul() const;

  // Check quickly if buffer looks like it contains binary data
  bool is_binary() const;

  // Get string representation of data buffer
  std::string to_string() const;

  // Access libgit2 C ptr
  git_buf *c_ptr();
  const git_buf *c_ptr() const;

private:
  git_buf c_struct_;
};

} // namespace cppgit2
