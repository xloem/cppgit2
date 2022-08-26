#include <cppgit2/repository.hpp>

namespace cppgit2 {

submodule::submodule() : c_ptr_(nullptr), owner_(ownership::libgit2) {}

submodule::submodule(git_submodule *c_ptr, ownership owner)
    : c_ptr_(c_ptr), owner_(owner) {}

submodule::~submodule() {
  if (c_ptr_ && owner_ == ownership::user)
    git_submodule_free(c_ptr_);
}

submodule::submodule(submodule&& other) : c_ptr_(other.c_ptr_), owner_(other.owner_) {
  other.c_ptr_ = nullptr;
}

submodule& submodule::operator=(submodule&& other) {
  if (other.c_ptr_ != c_ptr_) {
    c_ptr_ = other.c_ptr_;
    owner_ = other.owner_;
    other.c_ptr_ = nullptr;
  }
  return *this;
}

void submodule::init(bool overwrite) {
  git_exception::throw_nonzero(
    git_submodule_init(c_ptr_, overwrite));
}

void submodule::resolve_setup() {
  git_exception::throw_nonzero(
    git_submodule_add_finalize(c_ptr_));
}

void submodule::add_to_index(bool write_index) {
  git_exception::throw_nonzero(
    git_submodule_add_to_index(c_ptr_, write_index));
}

std::string submodule::branch_name() const {
  auto ret = git_submodule_branch(c_ptr_);
  if (ret)
    return std::string(ret);
  else
    return "";
}

submodule::recurse submodule::recurse_submodules_option() const {
  return static_cast<submodule::recurse>(
      git_submodule_fetch_recurse_submodules(c_ptr_));
}

oid submodule::head_id() const { return oid(git_submodule_head_id(c_ptr_)); }

submodule::ignore submodule::ignore_option() const {
  return static_cast<submodule::ignore>(git_submodule_ignore(c_ptr_));
}

repository submodule::initialize_repository(bool use_gitlink) {
  repository result;
  git_exception::throw_nonzero(
    git_submodule_repo_init(&result.c_ptr_, c_ptr_, use_gitlink));
  return result;
}

submodule::update_strategy submodule::get_update_strategy() const {
  return static_cast<submodule::update_strategy>(
      git_submodule_update_strategy(c_ptr_));
}

oid submodule::index_id() const { return oid(git_submodule_index_id(c_ptr_)); }

std::string submodule::name() const {
  auto ret = git_submodule_name(c_ptr_);
  if (ret)
    return std::string(ret);
  else
    return "";
}

std::string submodule::path() const {
  auto ret = git_submodule_path(c_ptr_);
  if (ret)
    return std::string(ret);
  else
    return "";
}

void submodule::reload(bool force) {
  git_exception::throw_nonzero(
    git_submodule_reload(c_ptr_, force));
}

cppgit2::repository submodule::open_repository() {
  cppgit2::repository result;
  git_exception::throw_nonzero(
    git_submodule_open(&result.c_ptr_, c_ptr_));
  return result;
}

cppgit2::repository submodule::owner() {
  return repository(git_submodule_owner(c_ptr_));
}

void submodule::sync() {
  git_exception::throw_nonzero(
    git_submodule_sync(c_ptr_));
}

std::string submodule::url() const {
  auto ret = git_submodule_url(c_ptr_);
  if (ret)
    return std::string(ret);
  else
    return "";
}

submodule::status submodule::location_status() const {
  unsigned int result;
  git_exception::throw_nonzero(
    git_submodule_location(&result, c_ptr_));
  return static_cast<submodule::status>(result);
}

repository submodule::clone(const update_options &options) {
  repository result;
  git_exception::throw_nonzero(
    git_submodule_clone(&result.c_ptr_, c_ptr_, options.c_ptr_));
  return result;
}

void submodule::update(bool init, const update_options &options) {
  git_exception::throw_nonzero(
    git_submodule_update(c_ptr_, init, options.c_ptr_));
}

const git_submodule *submodule::c_ptr() const { return c_ptr_; }

} // namespace cppgit2
