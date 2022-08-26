#include <cppgit2/repository.hpp>

namespace cppgit2 {
tag::tag() : c_ptr_(nullptr), owner_(ownership::libgit2) {}

tag::tag(git_tag *c_ptr, ownership owner) : c_ptr_(c_ptr), owner_(owner) {}

tag::~tag() {
  if (c_ptr_ && owner_ == ownership::user)
    git_tag_free(c_ptr_);
}

tag::tag(tag&& other) : c_ptr_(other.c_ptr_), owner_(other.owner_) {
  other.c_ptr_ = nullptr;
}

tag& tag::operator=(tag&& other) {
  if (other.c_ptr_ != c_ptr_) {
    c_ptr_ = other.c_ptr_;
    owner_ = other.owner_;
    other.c_ptr_ = nullptr;
  }
  return *this;
}

tag tag::copy() const {
  return *this;
}

tag::tag(tag const& other) : owner_(ownership::user){  
  git_exception::throw_nonzero(
    git_tag_dup(&c_ptr_, other.c_ptr_));
}

oid tag::id() const { return oid(git_tag_id(c_ptr_)); }

std::string tag::message() const {
  auto ret = git_tag_message(c_ptr_);
  if (ret)
    return std::string(ret);
  else
    return "";
}

std::string tag::name() const {
  auto ret = git_tag_name(c_ptr_);
  if (ret)
    return std::string(ret);
  else
    return "";
}

object tag::peel() const {
  object result(nullptr, ownership::user);
  git_exception::throw_nonzero(
    git_tag_peel(&result.c_ptr_, c_ptr_));
  return result;
}

signature tag::tagger() const { return signature(git_tag_tagger(c_ptr_)); }

object tag::target() const {
  object result(nullptr, ownership::user);
  git_exception::throw_nonzero(
    git_tag_target(&result.c_ptr_, c_ptr_));
  return result;
}

oid tag::target_id() const { return oid(git_tag_target_id(c_ptr_)); }

object::object_type tag::target_type() const {
  return static_cast<object::object_type>(git_tag_target_type(c_ptr_));
}

repository tag::owner() const { return repository(git_tag_owner(c_ptr_)); }

git_tag *tag::c_ptr() { return c_ptr_; }

const git_tag *tag::c_ptr() const { return c_ptr_; }

} // namespace cppgit2
