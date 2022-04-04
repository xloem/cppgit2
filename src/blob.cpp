#include <cppgit2/repository.hpp>

namespace cppgit2 {

blob::blob() : c_ptr_(nullptr) {}

blob::blob(const git_blob *c_ptr) {
  if (git_blob_dup(&c_ptr_, const_cast<git_blob *>(c_ptr)))
    throw git_exception();
}

blob::~blob() {
  if (c_ptr_)
    git_blob_free(c_ptr_);
}

blob::blob(blob&& other) : c_ptr_(other.c_ptr_) {
  other.c_ptr_ = nullptr;
}

blob& blob::operator=(blob&& other) {
  if (other.c_ptr_ != c_ptr_) {
    c_ptr_ = other.c_ptr_;
    other.c_ptr_ = nullptr;
  }
  return *this;
}

repository blob::owner() const { return repository(git_blob_owner(c_ptr_)); }

blob blob::copy() const {
  return *this;
}

blob::blob(blob const& other) {
  if (git_blob_dup(&c_ptr_, other.c_ptr_))
    throw git_exception();
}
blob& blob::operator=(const blob &other) {
  if (this == &other)
      return *this;

  if (c_ptr_){
    git_blob_free(c_ptr_);
  }
  if (git_blob_dup(&c_ptr_, const_cast<git_blob *>(other.c_ptr_)))
    throw git_exception();
  return *this;
}

oid blob::id() const { return oid(git_blob_id(c_ptr_)); }

bool blob::is_binary() const { return git_blob_is_binary(c_ptr_); }

const void *blob::raw_contents() const { return git_blob_rawcontent(c_ptr_); }

blob_size blob::raw_size() const { return git_blob_rawsize(c_ptr_); }

const git_blob *blob::c_ptr() const { return c_ptr_; }

} // namespace cppgit2
