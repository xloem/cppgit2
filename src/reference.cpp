#include <cppgit2/repository.hpp>

namespace cppgit2 {

reference::reference() : c_ptr_(nullptr), owner_(ownership::libgit2) {}

reference::reference(git_reference *c_ptr, ownership owner)
    : c_ptr_(c_ptr), owner_(owner) {}

reference::~reference() {
  if (c_ptr_ && owner_ == ownership::user)
    git_reference_free(c_ptr_);
}

reference::reference(reference&& other) : c_ptr_(other.c_ptr_), owner_(other.owner_) {
  other.c_ptr_ = nullptr;
}

reference& reference::operator=(reference&& other) {
  if (other.c_ptr_ != c_ptr_) {
    c_ptr_ = other.c_ptr_;
    owner_ = other.owner_;
    other.c_ptr_ = nullptr;
  }
  return *this;
}

int reference::compare(const reference &rhs) const {
  return git_reference_cmp(c_ptr_, rhs.c_ptr());
}

reference reference::copy() const {
  return *this;
}

reference::reference(reference const& other) : owner_(ownership::user){
  git_exception::throw_nonzero(
    git_reference_dup(&c_ptr_, other.c_ptr_));
}

void reference::delete_reference(reference &ref) {
  git_exception::throw_nonzero(
    git_reference_delete(ref.c_ptr_));
}

bool reference::is_branch() const { return git_reference_is_branch(c_ptr_); }

bool reference::is_note() const { return git_reference_is_note(c_ptr_); }

bool reference::is_remote() const { return git_reference_is_remote(c_ptr_); }

bool reference::is_tag() const { return git_reference_is_tag(c_ptr_); }

bool reference::is_valid_name(const std::string &refname) {
  return git_reference_is_valid_name(refname.c_str());
}

std::string reference::name() const {
  auto ret = git_reference_name(c_ptr_);
  if (ret)
    return std::string(ret);
  else
    return "";
}

std::string reference::normalize_name(size_t length, const std::string &name,
                                      reference::format flags) {
  char *buffer = (char *)malloc(length * sizeof(char));
  git_exception::throw_nonzero(
    git_reference_normalize_name(buffer, length, name.c_str(),
                                 static_cast<unsigned int>(flags)));
  std::string result{""};
  if (buffer)
    result = std::string(buffer);
  free(buffer);
  return result;
}

std::string reference::shorthand_name() const {
  auto ret = git_reference_shorthand(c_ptr_);
  if (ret)
    return std::string(ret);
  else
    return "";
}

repository reference::owner() const {
  return repository(git_reference_owner(c_ptr_));
}

object reference::peel_until(object::object_type type) {
  object result(nullptr, ownership::user);
  git_exception::throw_nonzero(
    git_reference_peel(&result.c_ptr_, c_ptr_,
                       static_cast<git_object_t>(type)));
  return result;
}

reference reference::rename(const std::string &new_name, bool force,
                            const std::string &log_message) {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
    git_reference_rename(&result.c_ptr_, c_ptr_, new_name.c_str(), force,
                         log_message.c_str()));
  return result;
}

reference reference::resolve() {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
    git_reference_resolve(&result.c_ptr_, c_ptr_));
  return result;
}

reference reference::set_target(const oid &id, const std::string &log_message) {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
    git_reference_set_target(&result.c_ptr_, c_ptr_, id.c_ptr(),
                             log_message.c_str()));
  return result;
}

reference reference::set_symbolic_target(const std::string &target,
                                         const std::string &log_message) {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
    git_reference_symbolic_set_target(&result.c_ptr_, c_ptr_, target.c_str(),
                                      log_message.c_str()));
  return result;
}

oid reference::target() const { return oid(git_reference_target(c_ptr_)); }

std::string reference::symbolic_target() const {
  auto ret = git_reference_symbolic_target(c_ptr_);
  if (ret)
    return std::string(ret);
  else
    return "";
}

oid reference::peeled_target() const {
  return oid(git_reference_target_peel(c_ptr_));
}

reference::reference_type reference::type() const {
  return static_cast<reference_type>(git_reference_type(c_ptr_));
}

const git_reference *reference::c_ptr() const { return c_ptr_; }

} // namespace cppgit2
