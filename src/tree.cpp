#include <cppgit2/repository.hpp>
#include <functional>

namespace cppgit2 {

tree::tree() : c_ptr_(nullptr), owner_(ownership::libgit2) {}

tree::tree(git_tree *c_ptr, ownership owner) : c_ptr_(c_ptr), owner_(owner) {}

tree::~tree() {
  if (c_ptr_ && owner_ == ownership::user)
    git_tree_free(c_ptr_);
}

tree::tree(tree&& other) : c_ptr_(other.c_ptr_), owner_(other.owner_) {
  other.c_ptr_ = nullptr;
}

tree& tree::operator=(tree&& other) {
  if (other.c_ptr_ != c_ptr_) {
    c_ptr_ = other.c_ptr_;
    owner_ = other.owner_;
    other.c_ptr_ = nullptr;
  }
  return *this;
}

tree::entry tree::lookup_entry_by_id(const oid &id) const {
  return tree::entry(
      const_cast<git_tree_entry *>(git_tree_entry_byid(c_ptr_, id.c_ptr())));
}

tree::entry tree::lookup_entry_by_index(size_t index) const {
  return tree::entry(
      const_cast<git_tree_entry *>(git_tree_entry_byindex(c_ptr_, index)));
}

tree::entry tree::lookup_entry_by_name(const std::string &filename) const {
  return tree::entry(const_cast<git_tree_entry *>(
      git_tree_entry_byname(c_ptr_, filename.c_str())));
}

tree::entry tree::lookup_entry_by_path(const std::string &path) const {
  tree::entry result(nullptr, ownership::user);
  result.owner_ = ownership::user;
  git_exception::throw_nonzero(
    git_tree_entry_bypath(&result.c_ptr_, c_ptr_, path.c_str()));
  return result;
}

oid tree::id() const { return oid(git_tree_id(c_ptr_)); }

tree tree::copy() const {
  return *this;
}

tree::tree(tree const& other) : owner_(ownership::user){
  git_exception::throw_nonzero(
    git_tree_dup(&c_ptr_, other.c_ptr_));
}

size_t tree::size() const { return git_tree_entrycount(c_ptr_); }

repository tree::owner() const { return repository(git_tree_owner(c_ptr_)); }

void tree::walk(traversal_mode mode,
                std::function<int(const std::string &, const tree::entry &)>
                    visitor) const {
  struct visitor_wrapper {
    std::function<int(const std::string &, const tree::entry &)> fn;
  };

  visitor_wrapper wrapper;
  wrapper.fn = visitor;

  auto callback_c = [](const char *root, const git_tree_entry *entry,
                       void *payload) {
    auto wrapper = reinterpret_cast<visitor_wrapper *>(payload);
    return wrapper->fn(root ? std::string(root) : "", tree::entry(entry));
  };

  git_exception::throw_nonzero(
    git_tree_walk(c_ptr_, static_cast<git_treewalk_mode>(mode), callback_c,
                  (void *)(&wrapper)));
}

git_tree *tree::c_ptr() { return c_ptr_; }

const git_tree *tree::c_ptr() const { return c_ptr_; }

} // namespace cppgit2
