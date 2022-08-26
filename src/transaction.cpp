#include <cppgit2/transaction.hpp>

namespace cppgit2 {

transaction::transaction() : c_ptr_(nullptr), owner_(ownership::libgit2) {}

transaction::transaction(git_transaction *c_ptr, ownership owner)
    : c_ptr_(c_ptr), owner_(owner) {}

transaction::~transaction() {
  if (c_ptr_ && owner_ == ownership::user)
    git_transaction_free(c_ptr_);
}

transaction::transaction(transaction&& other) : c_ptr_(other.c_ptr_), owner_(other.owner_) {
  other.c_ptr_ = nullptr;
}

transaction& transaction::operator=(transaction&& other) {
  if (other.c_ptr_ != c_ptr_) {
    c_ptr_ = other.c_ptr_;
    owner_ = other.owner_;
    other.c_ptr_ = nullptr;
  }
  return *this;
}

void transaction::commit() {
  git_exception::throw_nonzero(
    git_transaction_commit(c_ptr_));
}

void transaction::lock_reference(const std::string &refname) {
  git_exception::throw_nonzero(
    git_transaction_lock_ref(c_ptr_, refname.c_str()));
}

void transaction::remove_reference(const std::string &refname) {
  git_exception::throw_nonzero(
    git_transaction_remove(c_ptr_, refname.c_str()));
}

void transaction::set_reflog(const std::string &refname, const reflog &reflog) {
  git_exception::throw_nonzero(
    git_transaction_set_reflog(c_ptr_, refname.c_str(), reflog.c_ptr()));
}

void transaction::set_symbolic_target(const std::string &refname,
                                      const std::string &target,
                                      const signature &signature,
                                      const std::string &message) {
  git_exception::throw_nonzero(
    git_transaction_set_symbolic_target(c_ptr_, refname.c_str(),
                                        target.c_str(), signature.c_ptr(),
                                        message.c_str()));
}

void transaction::set_target(const std::string &refname, const oid &target,
                             const signature &signature,
                             const std::string &message) {
  git_exception::throw_nonzero(
    git_transaction_set_target(c_ptr_, refname.c_str(), target.c_ptr(),
                               signature.c_ptr(), message.c_str()));
}

git_transaction *transaction::c_ptr() { return c_ptr_; }

const git_transaction *transaction::c_ptr() const { return c_ptr_; }

} // namespace cppgit2
