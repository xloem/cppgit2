#include <cppgit2/tree_builder.hpp>
#include <functional>

namespace cppgit2 {

tree_builder::tree_builder(repository &repo, tree source) {
  git_exception::throw_nonzero(
    git_treebuilder_new(&c_ptr_, repo.c_ptr_, source.c_ptr()));
}

tree_builder::~tree_builder() {
  if (c_ptr_)
    git_treebuilder_free(c_ptr_);
}

tree_builder::tree_builder(tree_builder&& other) : c_ptr_(other.c_ptr_) {
  other.c_ptr_ = nullptr;
}

tree_builder& tree_builder::operator=(tree_builder&& other) {
  if (other.c_ptr_ != c_ptr_) {
    c_ptr_ = other.c_ptr_;
    other.c_ptr_ = nullptr;
  }
  return *this;
}

void tree_builder::clear() { git_treebuilder_clear(c_ptr_); }

void tree_builder::filter(std::function<int(const tree::entry &)> visitor) {
  // Wrap user-provided visitor funciton in a struct
  struct visitor_wrapper {
    std::function<int(const tree::entry &)> fn;
  };

  visitor_wrapper wrapper;
  wrapper.fn = visitor;

  auto visitor_c = [](const git_tree_entry *entry, void *payload) {
    auto wrapped = reinterpret_cast<visitor_wrapper *>(payload);
    const tree::entry entry_arg = tree::entry(entry);
    return wrapped->fn(entry_arg);
  };

  git_treebuilder_filter(c_ptr_, visitor_c, (void *)(&wrapper));
}

size_t tree_builder::size() const { return git_treebuilder_entrycount(c_ptr_); }

tree::entry tree_builder::operator[](const std::string &filename) const {
  return tree::entry(const_cast<git_tree_entry *>(
                         git_treebuilder_get(c_ptr_, filename.c_str())),
                     ownership::libgit2);
}

void tree_builder::insert(const std::string &filename, const oid &id,
                          file_mode mode) {
  const git_tree_entry *result = nullptr;
  git_exception::throw_nonzero(
    git_treebuilder_insert(&result, c_ptr_, filename.c_str(), id.c_ptr(),
                           static_cast<git_filemode_t>(mode)));
}

void tree_builder::remove(const std::string &filename) {
  git_exception::throw_nonzero(
    git_treebuilder_remove(c_ptr_, filename.c_str()));
}

oid tree_builder::write() {
  git_oid id;
  git_exception::throw_nonzero(
    git_treebuilder_write(&id, c_ptr_));
  return oid(&id);
}

oid tree_builder::write(data_buffer &tree) {
  git_oid id;
  git_exception::throw_nonzero(
    git_treebuilder_write_with_buffer(&id, c_ptr_, tree.c_ptr()));
  return oid(&id);
}

const git_treebuilder *tree_builder::c_ptr() const { return c_ptr_; }

} // namespace cppgit2
