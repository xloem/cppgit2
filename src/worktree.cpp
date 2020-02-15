#include <cppgit2/worktree.hpp>

namespace cppgit2 {

worktree::worktree() : c_ptr_(nullptr), owner_(ownership::libgit2) {
  git_libgit2_init();
}

worktree::worktree(git_worktree *c_ptr, ownership owner)
    : c_ptr_(c_ptr), owner_(owner) {
  git_libgit2_init();
}

worktree::~worktree() {
  if (c_ptr_ && owner_ == ownership::user)
    git_worktree_free(c_ptr_);
  git_libgit2_shutdown();
}

std::pair<bool, std::string> worktree::is_locked() const {
  data_buffer result(nullptr);
  auto ret = git_worktree_is_locked(result.c_ptr(), c_ptr_);
  if (ret > 0) {
    // Locked
    if (result.c_ptr())
      return {true, result.to_string()};
    else
      return {true, ""};
  } else if (ret == 0) {
    // Not locked
    return {false, ""};
  } else {
    throw exception();
  }
}

bool worktree::is_prunable(unsigned int version, uint32_t flags) const {
  git_worktree_prune_options options{.version = version, .flags = flags};
  auto ret = git_worktree_is_prunable(c_ptr_, &options);
  if (ret == 1)
    return true;
  else if (ret == 0)
    return false;
  else
    throw exception();
}

bool worktree::is_prunable() const {
  git_worktree_prune_options options;
  // Initializes a git_worktree_prune_options with default values. Equivalent to
  // creating an instance with GIT_WORKTREE_PRUNE_OPTIONS_INIT.
  auto ret = git_worktree_prune_init_options(
      &options, GIT_WORKTREE_PRUNE_OPTIONS_VERSION);
  if (ret == 0) {
    ret = git_worktree_is_prunable(c_ptr_, &options);
    if (ret == 1)
      return true;
    else if (ret == 0)
      return false;
    else
      throw exception();
  } else
    throw exception();
}

void worktree::lock(const std::string &reason) {
  const char *reason_c = reason == "" ? NULL : reason.c_str();
  if (git_worktree_lock(c_ptr_, reason_c))
    throw exception();
}

std::string worktree::name() const {
  auto ret = git_worktree_name(c_ptr_);
  if (ret)
    return std::string(ret);
  else
    return "";
}

std::string worktree::path() const {
  auto ret = git_worktree_path(c_ptr_);
  if (ret)
    return std::string(ret);
  else
    return "";
}

void worktree::prune(unsigned int version, uint32_t flags) {
  git_worktree_prune_options options{.version = version, .flags = flags};
  if (git_worktree_prune(c_ptr_, &options))
    throw exception();
}

void worktree::prune() {
  git_worktree_prune_options options;
  // Initializes a git_worktree_prune_options with default values. Equivalent to
  // creating an instance with GIT_WORKTREE_PRUNE_OPTIONS_INIT.
  auto ret = git_worktree_prune_init_options(
      &options, GIT_WORKTREE_PRUNE_OPTIONS_VERSION);
  if (ret == 0) {
    if (git_worktree_prune(c_ptr_, &options))
      throw exception();
  } else
    throw exception();
}

void worktree::unlock() {
  if (git_worktree_unlock(c_ptr_))
    throw exception();
}

bool worktree::is_valid() { return git_worktree_validate(c_ptr_) == 0; }

bool worktree::validate() {
  if (!is_valid())
    throw exception();
  return true;
}

const git_worktree *worktree::c_ptr() const { return c_ptr_; }

} // namespace cppgit2