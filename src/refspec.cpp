#include <cppgit2/refspec.hpp>
using namespace cppgit2;

refspec::refspec() : c_ptr_(nullptr), owner_(ownership::libgit2) {}

refspec::refspec(git_refspec *c_ptr, ownership owner)
    : c_ptr_(c_ptr), owner_(owner) {}

refspec::~refspec() {
  if (c_ptr_ && owner_ == ownership::user)
    git_refspec_free(c_ptr_);
}

refspec::refspec(refspec&& other) : c_ptr_(other.c_ptr_), owner_(other.owner_) {
  other.c_ptr_ = nullptr;
}

refspec& refspec::operator=(refspec&& other) {
  if (other.c_ptr_ != c_ptr_) {
    c_ptr_ = other.c_ptr_;
    owner_ = other.owner_;
    other.c_ptr_ = nullptr;
  }
  return *this;
}

connection_direction refspec::direction() const {
  return static_cast<connection_direction>(git_refspec_direction(c_ptr_));
}

std::string refspec::destination() const {
  auto ret = git_refspec_dst(c_ptr_);
  return ret ? std::string(ret) : "";
}

bool refspec::destination_matches_reference(const std::string &refname) const {
  return git_refspec_dst_matches(c_ptr_, refname.c_str());
}

bool refspec::is_force_update_enabled() const {
  return git_refspec_force(c_ptr_);
}

refspec refspec::parse(const std::string &input, bool is_fetch) {
  refspec result(nullptr, ownership::user);
  git_exception::throw_nonzero(
    git_refspec_parse(&result.c_ptr_, input.c_str(), is_fetch));
  return result;
}

data_buffer
refspec::transform_target_to_source_reference(const std::string &name) {
  git_buf result = GIT_BUF_INIT;
  git_exception::throw_nonzero(
    git_refspec_rtransform(&result, c_ptr_, name.c_str()));
  return data_buffer(&result);
}

std::string refspec::source() const {
  auto ret = git_refspec_src(c_ptr_);
  return ret ? std::string(ret) : "";
}

bool refspec::source_matches_reference(const std::string &refname) const {
  return git_refspec_src_matches(c_ptr_, refname.c_str());
}

std::string refspec::to_string() const {
  auto ret = git_refspec_string(c_ptr_);
  return ret ? std::string(ret) : "";
}

data_buffer refspec::transform_reference(const std::string &name) {
  git_buf result;
  git_exception::throw_nonzero(
    git_refspec_transform(&result, c_ptr_, name.c_str()));
  return data_buffer(&result);
}

const git_refspec *refspec::c_ptr() const { return c_ptr_; }
