#include <cppgit2/reflog.hpp>

namespace cppgit2 {

reflog::reflog() : c_ptr_(nullptr), owner_(ownership::libgit2) {}

reflog::reflog(git_reflog *c_ptr, ownership owner)
    : c_ptr_(c_ptr), owner_(owner) {}

reflog::~reflog() {
  if (c_ptr_ && owner_ == ownership::user)
    git_reflog_free(c_ptr_);
}

reflog::reflog(reflog&& other) : c_ptr_(other.c_ptr_), owner_(other.owner_) {
  other.c_ptr_ = nullptr;
}

reflog& reflog::operator=(reflog&& other) {
  if (other.c_ptr_ != c_ptr_) {
    c_ptr_ = other.c_ptr_;
    owner_ = other.owner_;
    other.c_ptr_ = nullptr;
  }
  return *this;
}

void reflog::remove(size_t index, bool rewrite_previous_entry) {
  git_exception::throw_nonzero(
    git_reflog_drop(c_ptr_, index, rewrite_previous_entry));
}

void reflog::append(const oid &id, const signature &committer,
                    const std::string &message) {
  git_exception::throw_nonzero(
    git_reflog_append(c_ptr_, id.c_ptr(), committer.c_ptr(), message.c_str()));
}

reflog::entry reflog::operator[](size_t index) const {
  return reflog::entry(git_reflog_entry_byindex(c_ptr_, index));
}

void reflog::write_to_disk() {
  git_exception::throw_nonzero(
    git_reflog_write(c_ptr_));
}

size_t reflog::size() const { return git_reflog_entrycount(c_ptr_); }

const git_reflog *reflog::c_ptr() const { return c_ptr_; }

} // namespace cppgit2
