#include <cppgit2/data_buffer.hpp>
#include <cppgit2/repository.hpp>

namespace cppgit2 {

object::object() : c_ptr_(nullptr), owner_(ownership::user) {}

object::object(git_object *c_ptr, ownership owner)
    : c_ptr_(c_ptr), owner_(owner) {}

object::~object() {
  if (c_ptr_ && owner_ == ownership::user)
    git_object_free(c_ptr_);
}

object::object(object&& other) : c_ptr_(other.c_ptr_), owner_(other.owner_) {
  other.c_ptr_ = nullptr;
}

object& object::operator=(object&& other) {
  if (other.c_ptr_ != c_ptr_) {
    if (c_ptr_ && owner_ == ownership::user)
      git_object_free(c_ptr_);

    c_ptr_ = other.c_ptr_;
    owner_ = other.owner_;
    other.c_ptr_ = nullptr;
  }
  return *this;
}

oid object::id() const { return oid(git_object_id(c_ptr_)->id); }

std::string object::short_id() const {
  git_buf result;
  git_exception::throw_nonzero(
    git_object_short_id(&result, c_ptr_));
  return data_buffer(&result).to_string();
}

object object::copy() const {
  return *this;
}

object::object(object const& other) : owner_(ownership::user){
  git_exception::throw_nonzero(
    git_object_dup(&c_ptr_, other.c_ptr_));
}

std::string object::object_type_to_string(object_type type) {
  auto ret = git_object_type2string(static_cast<git_object_t>(type));
  return ret ? std::string(ret) : "";
}

object object::peel_until(object_type target_type) {
  object result(nullptr, ownership::user);
  git_exception::throw_nonzero(
    git_object_peel(&result.c_ptr_, c_ptr_,
                    static_cast<git_object_t>(target_type)));
  return result;
}

object::object_type object::type() const {
  return static_cast<object_type>(git_object_type(c_ptr_));
}

std::string object::type_string() const {
  return object::string_from_type(this->type());
}

object::object_type object::type_from_string(const std::string &type_string) {
  return static_cast<object_type>(git_object_string2type(type_string.c_str()));
}

std::string object::string_from_type(object_type type) {
  return std::string(git_object_type2string(static_cast<git_object_t>(type)));
}

bool object::is_type_loose(object_type type) {
  return git_object_typeisloose(static_cast<git_object_t>(type));
}

repository object::owner() const {
  return repository(git_object_owner(c_ptr_));
}

blob object::as_blob() {
  if (type() == object::object_type::blob)
    return blob((git_blob *)c_ptr_);
  else
    throw git_exception("object is not a blob",
                        git_exception::error_class::invalid,
                        git_exception::error_code::invalid);
}

commit object::as_commit() {
  if (type() == object::object_type::commit)
    return commit((git_commit *)c_ptr_);
  else
    throw git_exception("object is not a commit",
                        git_exception::error_class::invalid,
                        git_exception::error_code::invalid);
}

tree object::as_tree() {
  if (type() == object::object_type::tree)
    return tree((git_tree *)c_ptr_);
  else
    throw git_exception("object is not a tree",
                        git_exception::error_class::invalid,
                        git_exception::error_code::invalid);
}

tag object::as_tag() {
  if (type() == object::object_type::tag)
    return tag((git_tag *)c_ptr_);
  else
    throw git_exception("object is not a tag",
                        git_exception::error_class::invalid,
                        git_exception::error_code::invalid);
}

} // namespace cppgit2
