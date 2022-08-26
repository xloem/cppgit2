#include <cppgit2/repository.hpp>
#include <functional>

namespace cppgit2 {

index::index() : c_ptr_(nullptr), owner_(ownership::user) {
  git_index_new(&c_ptr_); // owned by user
}

index::index(git_index *c_ptr, ownership owner)
    : c_ptr_(c_ptr), owner_(owner) {}

index::~index() {
  if (c_ptr_ && owner_ == ownership::user)
    git_index_free(c_ptr_);
}

index::index(index&& other) : c_ptr_(other.c_ptr_), owner_(other.owner_) {
  other.c_ptr_ = nullptr;
}

index& index::operator=(index&& other) {
  if (other.c_ptr_ != c_ptr_) {
    c_ptr_ = other.c_ptr_;
    owner_ = other.owner_;
    other.c_ptr_ = nullptr;
  }
  return *this;
}

void index::add_entry(const entry &source_entry) {
  git_exception::throw_nonzero(
    git_index_add(c_ptr_, source_entry.c_ptr()));
}

void index::add_entries_that_match(
    const std::vector<std::string> &pathspec, add_option flags,
    std::function<int(const std::string &, const std::string &)> callback) {

  struct callback_wrapper {
    std::function<int(const std::string &, const std::string &)> fn;
  };

  callback_wrapper wrapper;
  wrapper.fn = callback;

  auto callback_c = [](const char *pathspec, const char *matched_pathspec,
                       void *payload) {
    auto wrapper = reinterpret_cast<callback_wrapper *>(payload);
    return wrapper->fn(pathspec, matched_pathspec);
  };

  auto pathspec_c = strarray(pathspec).c_ptr();
  git_exception::throw_nonzero(
    git_index_add_all(c_ptr_, pathspec_c, static_cast<int>(flags), callback_c,
                      (void *)(&wrapper)));
}

void index::add_entry_by_path(const std::string &path) {
  git_exception::throw_nonzero(
    git_index_add_bypath(c_ptr_, path.c_str()));
}

void index::add_entry_from_buffer(const entry &entry,
                                  const std::string &buffer) {
  git_exception::throw_nonzero(
    git_index_add_frombuffer(c_ptr_, entry.c_ptr(), buffer.c_str(),
                             buffer.size()));
}

index::capability index::capability_flags() const {
  return static_cast<capability>(git_index_caps(c_ptr_));
}

const oid index::checksum() { return oid(git_index_checksum(c_ptr_)); }

void index::clear() {
  git_exception::throw_nonzero(
    git_index_clear(c_ptr_));
}

void index::add_conflict_entry(const entry &ancestor_entry,
                               const entry &our_entry,
                               const entry &their_entry) {
  git_exception::throw_nonzero(
    git_index_conflict_add(c_ptr_, ancestor_entry.c_ptr(), our_entry.c_ptr(),
                           their_entry.c_ptr()));
}

void index::remove_all_conflicts() {
  git_exception::throw_nonzero(
    git_index_conflict_cleanup(c_ptr_));
}

void index::remove_conflict_entries(const std::string &path) {
  git_exception::throw_nonzero(
    git_index_conflict_remove(c_ptr_, path.c_str()));
}

size_t index::size() const { return git_index_entrycount(c_ptr_); }

size_t index::find_first(const std::string &path) {
  size_t result{0};
  git_exception::throw_nonzero(
    git_index_find(&result, c_ptr_, path.c_str()));
  return result;
}

size_t index::find_first_matching_prefix(const std::string &prefix) {
  size_t result{0};
  git_exception::throw_nonzero(
    git_index_find_prefix(&result, c_ptr_, prefix.c_str()));
  return result;
}

void index::for_each(std::function<void(const index::entry &)> visitor) {
  git_index_iterator *iter;
  git_index_iterator_new(&iter, c_ptr_);
  const git_index_entry *entry_c;
  int ret;
  while ((ret = git_index_iterator_next(&entry_c, iter)) == 0) {
    entry payload(nullptr);
    *(payload.c_ptr_) = *(entry_c);
    visitor(payload);
  }
  git_index_iterator_free(iter);
}

void index::for_each_conflict(
    std::function<void(const entry &, const entry &, const entry &)> visitor) {
  git_index_conflict_iterator *iter;
  git_index_conflict_iterator_new(&iter, c_ptr_);
  const git_index_entry *ancestor_out, *our_out, *their_out;
  int ret;
  while ((ret = git_index_conflict_next(&ancestor_out, &our_out, &their_out,
                                        iter)) == 0) {
    entry ancestor_payload(nullptr), our_payload(nullptr),
        their_payload(nullptr);
    *(ancestor_payload.c_ptr_) = *(ancestor_out);
    *(our_payload.c_ptr_) = *(our_out);
    *(their_payload.c_ptr_) = *(their_out);
    visitor(ancestor_payload, our_payload, their_payload);
  }
  git_index_conflict_iterator_free(iter);
}

const index::entry index::operator[](size_t index) {
  auto ret = git_index_get_byindex(c_ptr_, index);
  if (!ret)
    throw git_exception();
  entry result(nullptr);
  if (ret)
    *(result.c_ptr_) = *ret;
  return result;
}

const index::entry index::entry_in_path(const std::string &path, stage s) {
  auto ret = git_index_get_bypath(c_ptr_, path.c_str(), static_cast<int>(s));
  if (!ret)
    throw git_exception();
  entry result(nullptr);
  *(result.c_ptr_) = *ret;
  return result;
}

bool index::has_conflicts() const { return git_index_has_conflicts(c_ptr_); }

std::string index::path() const {
  auto ret = git_index_path(c_ptr_);
  if (ret)
    return std::string(ret);
  else
    return "";
}

void index::read(bool force) {
  git_exception::throw_nonzero(
    git_index_read(c_ptr_, force));
}

void index::read_tree(const tree &tree) {
  git_exception::throw_nonzero(
    git_index_read_tree(c_ptr_, tree.c_ptr()));
}

void index::remove_entry(const std::string &path, index::stage stage) {
  git_exception::throw_nonzero(
    git_index_remove(c_ptr_, path.c_str(), static_cast<int>(stage)));
}

void index::remove_entries_that_match(
    const std::vector<std::string> &pathspec,
    std::function<int(const std::string &, const std::string &)> callback) {
  struct callback_wrapper {
    std::function<int(const std::string &, const std::string &)> fn;
  };

  callback_wrapper wrapper;
  wrapper.fn = callback;

  auto callback_c = [](const char *pathspec, const char *matched_pathspec,
                       void *payload) {
    auto wrapper = reinterpret_cast<callback_wrapper *>(payload);
    return wrapper->fn(pathspec, matched_pathspec);
  };

  auto pathspec_c = strarray(pathspec).c_ptr();
  git_exception::throw_nonzero(
    git_index_remove_all(c_ptr_, pathspec_c, callback_c, (void *)(&wrapper)));
}

void index::remove_entry_by_path(const std::string &path) {
  git_exception::throw_nonzero(
    git_index_remove_bypath(c_ptr_, path.c_str()));
}

void index::remove_entries_in_directory(const std::string &dir, stage stage) {
  git_exception::throw_nonzero(
    git_index_remove_directory(c_ptr_, dir.c_str(), static_cast<int>(stage)));
}

void index::set_index_capabilities(capability caps) {
  git_exception::throw_nonzero(
    git_index_set_caps(c_ptr_, static_cast<int>(caps)));
}

void index::set_version(unsigned int version) {
  git_exception::throw_nonzero(
    git_index_set_version(c_ptr_, version));
}

void index::update_entries_that_match(
    const std::vector<std::string> &pathspec,
    std::function<int(const std::string &, const std::string &)> callback) {
  struct callback_wrapper {
    std::function<int(const std::string &, const std::string &)> fn;
  };

  callback_wrapper wrapper;
  wrapper.fn = callback;

  auto callback_c = [](const char *pathspec, const char *matched_pathspec,
                       void *payload) {
    auto wrapper = reinterpret_cast<callback_wrapper *>(payload);
    return wrapper->fn(pathspec, matched_pathspec);
  };

  auto pathspec_c = strarray(pathspec).c_ptr();
  git_exception::throw_nonzero(
    git_index_update_all(c_ptr_, pathspec_c, callback_c, (void *)(&wrapper)));
}

unsigned int index::version() const { return git_index_version(c_ptr_); }

void index::write() {
  git_exception::throw_nonzero(
    git_index_write(c_ptr_));
}

oid index::write_tree() {
  oid result;
  git_exception::throw_nonzero(
    git_index_write_tree(result.c_ptr(), c_ptr_));
  return result;
}

oid index::write_tree_to(const repository &repo) {
  oid result;
  git_exception::throw_nonzero(
    git_index_write_tree_to(result.c_ptr(), c_ptr_, repo.c_ptr_));
  return result;
}

index index::open(const std::string &path) {
  index result(nullptr, ownership::user);
  git_exception::throw_nonzero(
    git_index_open(&result.c_ptr_, path.c_str()));
  return result;
}

repository index::owner() const { return repository(git_index_owner(c_ptr_)); }

const git_index *index::c_ptr() const { return c_ptr_; }

} // namespace cppgit2
