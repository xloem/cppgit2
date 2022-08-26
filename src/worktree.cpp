#include <cppgit2/worktree.hpp>

namespace cppgit2 {

worktree::worktree() : c_ptr_(nullptr), owner_(ownership::libgit2) {}

worktree::worktree(git_worktree *c_ptr, ownership owner)
    : c_ptr_(c_ptr), owner_(owner) {}

worktree::~worktree() {
  if (c_ptr_ && owner_ == ownership::user)
    git_worktree_free(c_ptr_);
}

worktree::worktree(worktree&& other) : c_ptr_(other.c_ptr_), owner_(other.owner_) {
  other.c_ptr_ = nullptr;
}

worktree& worktree::operator=(worktree&& other) {
  if (other.c_ptr_ != c_ptr_) {
    c_ptr_ = other.c_ptr_;
    owner_ = other.owner_;
    other.c_ptr_ = nullptr;
  }
  return *this;
}

std::pair<bool, std::string> worktree::is_locked() const {
  git_buf buf = GIT_BUF_INIT;
  auto ret = git_worktree_is_locked(&buf, c_ptr_);
  if (ret > 0) {
    // Locked
    if (buf.size) // size > 0 => reason available
      return std::pair<bool, std::string>{true, data_buffer(&buf).to_string()};
    else
      return std::pair<bool, std::string>{true, ""};
  } else if (ret == 0) {
    // Not locked
    return std::pair<bool, std::string>{false, ""};
  } else {
    throw git_exception(ret);
  }
}

bool worktree::is_prunable(unsigned int version, uint32_t flags) const {
  git_worktree_prune_options options;
  options.version = version;
  options.flags = flags;
  return git_exception::throw_nonbool(
    git_worktree_is_prunable(c_ptr_, &options));
}

bool worktree::is_prunable() const {
  git_worktree_prune_options options;
  // Initializes a git_worktree_prune_options with default values. Equivalent to
  // creating an instance with GIT_WORKTREE_PRUNE_OPTIONS_INIT.
  git_exception::throw_nonzero(
    git_worktree_prune_options_init(
        &options, GIT_WORKTREE_PRUNE_OPTIONS_VERSION));
  return git_exception::throw_nonbool(
    git_worktree_is_prunable(c_ptr_, &options));
}

void worktree::lock(const std::string &reason) {
  const char *reason_c = reason.empty() ? nullptr : reason.c_str();
  git_exception::throw_nonzero(
    git_worktree_lock(c_ptr_, reason_c));
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
  git_worktree_prune_options options;
  options.version = version;
  options.flags = flags;
  git_exception::throw_nonzero(
    git_worktree_prune(c_ptr_, &options));
}

void worktree::prune() {
  git_worktree_prune_options options;
  // Initializes a git_worktree_prune_options with default values. Equivalent to
  // creating an instance with GIT_WORKTREE_PRUNE_OPTIONS_INIT.
  git_exception::throw_nonzero(
    git_worktree_prune_options_init(
        &options, GIT_WORKTREE_PRUNE_OPTIONS_VERSION));
  git_exception::throw_nonzero(
    git_worktree_prune(c_ptr_, &options));
}

void worktree::unlock() {
  git_exception::throw_nonzero(
    git_worktree_unlock(c_ptr_));
}

bool worktree::is_valid() { return git_worktree_validate(c_ptr_) == 0; }

bool worktree::validate() {
  if (!is_valid())
    throw git_exception();
  return true;
}

const git_worktree *worktree::c_ptr() const { return c_ptr_; }

} // namespace cppgit2
