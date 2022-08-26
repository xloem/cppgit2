#include <cppgit2/merge.hpp>
using namespace cppgit2;

merge::file::result merge::merge_files(const merge::file::input &ancestor,
                                       const merge::file::input &ours,
                                       const merge::file::input &theirs,
                                       const merge::file::options &options) {
  git_merge_file_result result;
  git_exception::throw_nonzero(
    git_merge_file(&result, ancestor.c_ptr(), ours.c_ptr(), theirs.c_ptr(),
                     options.c_ptr()));
  return merge::file::result(&result);
}
