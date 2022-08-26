#include <cppgit2/repository.hpp>
#include <functional>

namespace cppgit2 {

repository::repository(git_repository *c_ptr) : c_ptr_(c_ptr) {}

repository::~repository() {
  if (c_ptr_)
    git_repository_free(c_ptr_);
}

repository::repository(repository&& other) : c_ptr_(other.c_ptr_) {
  other.c_ptr_ = nullptr;
}

repository& repository::operator=(repository&& other) {
  if (other.c_ptr_ != c_ptr_) {
    c_ptr_ = other.c_ptr_;
    other.c_ptr_ = nullptr;
  }
  return *this;
}

repository repository::init(const std::string &path, bool is_bare) {
  repository result(nullptr);
  git_exception::throw_nonzero(
      git_repository_init(&result.c_ptr_, path.c_str(), is_bare));
  return result;
}

repository repository::init_ext(const std::string &repo_path,
                                const init_options &options) {
  repository result(nullptr);
  git_exception::throw_nonzero(
      git_repository_init_ext(&result.c_ptr_, repo_path.c_str(),
                              options.c_ptr_));
  return result;
}

repository repository::open(const std::string &path) {
  repository result(nullptr);
  git_exception::throw_nonzero(
      git_repository_open(&result.c_ptr_, path.c_str()));
  return result;
}

repository repository::open_bare(const std::string &path) {
  repository result(nullptr);
  git_exception::throw_nonzero(
      git_repository_open_bare(&result.c_ptr_, path.c_str()));
  return result;
}

repository repository::open_ext(const std::string &path, open_flag flags,
                                const std::string &ceiling_dirs) {
  repository result(nullptr);
  git_exception::throw_nonzero(
      git_repository_open_ext(&result.c_ptr_, path.c_str(),
                              static_cast<unsigned int>(flags),
                              ceiling_dirs.c_str()));
  return result;
}

repository repository::open_from_worktree(const worktree &wt) {
  repository result(nullptr);
  git_exception::throw_nonzero(
      git_repository_open_from_worktree(&result.c_ptr_, wt.c_ptr_));
  return result;
}

repository repository::clone(const std::string &url,
                             const std::string &local_path,
                             const clone::options &options) {
  repository result;
  git_exception::throw_nonzero(
      git_clone(&result.c_ptr_, url.c_str(), local_path.c_str(),
                options.c_ptr()));
  return result;
}

std::string repository::path() const {
  return std::string(git_repository_path(c_ptr_));
}

bool repository::is_bare() const { return git_repository_is_bare(c_ptr_); }

bool repository::is_empty() const {
  return git_exception::throw_nonbool(
      git_repository_is_empty(c_ptr_));
}

bool repository::is_shallow() const {
  return git_repository_is_shallow(c_ptr_);
}

bool repository::is_worktree() const {
  return git_repository_is_worktree(c_ptr_);
}

std::string repository::commondir() const {
  auto ret = git_repository_commondir(c_ptr_);
  if (ret)
    return std::string(ret);
  else
    throw git_exception();
}

cppgit2::config repository::config() const {
  cppgit2::config result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_repository_config(&result.c_ptr_, c_ptr_));
  return result;
}

cppgit2::config repository::config_snapshot() const {
  cppgit2::config result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_repository_config_snapshot(&result.c_ptr_, c_ptr_));
  return result;
}

void repository::detach_head() const {
  git_exception::throw_nonzero(
      git_repository_detach_head(c_ptr_));
}

std::string repository::discover_path(const std::string &start_path,
                                      bool across_fs,
                                      const std::string &ceiling_dirs) {
  git_buf buffer = GIT_BUF_INIT;
  git_exception::throw_nonzero(
      git_repository_discover(&buffer, start_path.c_str(), across_fs,
                              ceiling_dirs.c_str()));
  return data_buffer(&buffer).to_string();
}

std::string repository::discover_path(const std::string &start_path) {
  return discover_path(start_path, false, "");
}

void repository::for_each_fetch_head(
    std::function<void(const std::string &, const std::string &, const oid &,
                       bool)>
        visitor) const {
  struct visitor_wrapper {
    std::function<void(const std::string &, const std::string &, const oid &,
                       bool)>
        fn;
  };

  visitor_wrapper wrapper;
  wrapper.fn = visitor;

  auto callback_c = [](const char *ref_name, const char *remote_url,
                       const git_oid *oid_c, unsigned int is_merge,
                       void *payload) {
    auto wrapper = reinterpret_cast<visitor_wrapper *>(payload);
    wrapper->fn(ref_name, remote_url, oid(oid_c), is_merge);
    return 0;
  };

  git_exception::throw_nonzero(
      git_repository_fetchhead_foreach(c_ptr_, callback_c, (void *)(&wrapper)));
}

void repository::for_each_merge_head(std::function<void(const oid &)> visitor) const {
  struct visitor_wrapper {
    std::function<void(const oid &)> fn;
  };

  visitor_wrapper wrapper;
  wrapper.fn = visitor;

  auto callback_c = [](const git_oid *oid_c, void *payload) {
    auto wrapper = reinterpret_cast<visitor_wrapper *>(payload);
    wrapper->fn(oid(oid_c));
    return 0;
  };

  git_exception::throw_nonzero(
      git_repository_mergehead_foreach(c_ptr_, callback_c, (void *)(&wrapper)));
}

std::string repository::namespace_() const {
  auto ret = git_repository_get_namespace(c_ptr_);
  if (ret)
    return std::string(ret);
  else
    throw git_exception("namespace directory does not exist",
                        git_exception::error_class::invalid,
                        git_exception::error_code::invalid);
}

oid repository::hashfile(const std::string &path, object::object_type type,
                         const std::string &as_path) const {
  oid result;
  git_exception::throw_nonzero(
      git_repository_hashfile(result.c_ptr(), c_ptr_, path.c_str(),
                              static_cast<git_object_t>(type), as_path.c_str()));
  return result;
}

oid repository::hashfile(const std::string &path, object::object_type type) const {
  return hashfile(path, type, path);
}

reference repository::head() const {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_repository_head(&result.c_ptr_, c_ptr_));
  return result;
}

reference repository::head_for_worktree(const std::string &name) const {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_repository_head_for_worktree(&result.c_ptr_, c_ptr_, name.c_str()));
  return result;
}

bool repository::is_head_detached() const {
  return git_exception::throw_nonbool(
      git_repository_head_detached(c_ptr_));
}

bool repository::is_head_detached_for_worktree(const std::string &path) {
  return git_exception::throw_nonbool(
      git_repository_head_detached_for_worktree(c_ptr_, path.c_str()));
}

bool repository::is_head_unborn() const {
  return git_exception::throw_nonbool(
      git_repository_head_unborn(c_ptr_));
}

std::pair<std::string, std::string> repository::identity() const {
  const char *name_c, *email_c;
  git_exception::throw_nonzero(
      git_repository_ident(&name_c, &email_c, c_ptr_));
  return std::pair<std::string, std::string>{name_c ? name_c : "",
                                             email_c ? email_c : ""};
}

cppgit2::index repository::index() const {
  cppgit2::index result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_repository_index(&result.c_ptr_, c_ptr_));
  return result;
}

std::string repository::path(repository::item item) const {
  git_buf buffer = GIT_BUF_INIT;
  git_exception::throw_nonzero(
      git_repository_item_path(&buffer, c_ptr_,
                               static_cast<git_repository_item_t>(item)));
  return data_buffer(&buffer).to_string();
}

std::string repository::message() const {
  git_buf buffer = GIT_BUF_INIT;
  git_exception::throw_nonzero(
      git_repository_message(&buffer, c_ptr_));
    
  return data_buffer(&buffer).to_string();
}

void repository::remove_message() const { git_repository_message_remove(c_ptr_); }

cppgit2::odb repository::odb() const {
  cppgit2::odb result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_repository_odb(&result.c_ptr_, c_ptr_));
  return result;
}

cppgit2::refdb repository::refdb() const {
  cppgit2::refdb result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_repository_refdb(&result.c_ptr_, c_ptr_));
  return result;
}

void repository::set_head(const std::string &refname) const {
  git_exception::throw_nonzero(
      git_repository_set_head(c_ptr_, refname.c_str()));
}

void repository::set_head_detached(const oid &commitish) const {
  git_exception::throw_nonzero(
      git_repository_set_head_detached(c_ptr_, commitish.c_ptr()));
}

void repository::set_head_detached(const annotated_commit &commitish) const {
  git_exception::throw_nonzero(
      git_repository_set_head_detached_from_annotated(c_ptr_, commitish.c_ptr_));
}

void repository::set_identity(const std::string &name,
                              const std::string &email) const {
  git_exception::throw_nonzero(
      git_repository_set_ident(c_ptr_, name.c_str(), email.c_str()));
}

void repository::unset_identity() const {
  git_exception::throw_nonzero(
      git_repository_set_ident(c_ptr_, nullptr, nullptr));
}

void repository::set_namespace(const std::string &namespace_) const {
  git_exception::throw_nonzero(
      git_repository_set_namespace(c_ptr_, namespace_.c_str()));
}

void repository::set_workdir(const std::string &workdir, bool update_gitlink) const {
  git_exception::throw_nonzero(
      git_repository_set_workdir(c_ptr_, workdir.c_str(), update_gitlink));
}

void repository::cleanup_state() const {
  git_exception::throw_nonzero(
      git_repository_state_cleanup(c_ptr_));
}

repository::repository_state repository::state() const {
  switch (git_repository_state(c_ptr_)) {
  case GIT_REPOSITORY_STATE_NONE:
    return repository_state::none;
  case GIT_REPOSITORY_STATE_REBASE_INTERACTIVE:
    return repository_state::rebase_interactive;
  case GIT_REPOSITORY_STATE_REBASE_MERGE:
    return repository_state::rebase_merge;
  case GIT_REPOSITORY_STATE_REBASE:
    return repository_state::rebase;
  case GIT_REPOSITORY_STATE_APPLY_MAILBOX:
    return repository_state::apply_mailbox;
  case GIT_REPOSITORY_STATE_APPLY_MAILBOX_OR_REBASE:
    return repository_state::apply_mailbox_or_rebase;
  case GIT_REPOSITORY_STATE_MERGE:
    return repository_state::merge;
  case GIT_REPOSITORY_STATE_REVERT:
    return repository_state::revert;
  case GIT_REPOSITORY_STATE_REVERT_SEQUENCE:
    return repository_state::revert_sequence;
  case GIT_REPOSITORY_STATE_CHERRYPICK:
    return repository_state::cherrypick;
  case GIT_REPOSITORY_STATE_CHERRYPICK_SEQUENCE:
    return repository_state::cherrypick_sequence;
  case GIT_REPOSITORY_STATE_BISECT:
    return repository_state::bisect;
  case -1:
  default:
    return repository_state::unknown;
  }
}

std::string repository::workdir() const {
  auto ret = git_repository_workdir(c_ptr_);
  if (ret) {
    return std::string(ret);
  } else {
    throw git_exception("working directory does not exist",
                        git_exception::error_class::invalid,
                        git_exception::error_code::invalid);
  }
}

repository repository::wrap_odb(const cppgit2::odb &odb) {
  repository result(nullptr);
  git_exception::throw_nonzero(
      git_repository_wrap_odb(&result.c_ptr_, odb.c_ptr_));
  return result;
}

const git_repository *repository::c_ptr() const { return c_ptr_; }

annotated_commit
repository::create_annotated_commit(const std::string &branch_name,
                                    const std::string &remote_url,
                                    const oid &id) const {
  annotated_commit result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_annotated_commit_from_fetchhead(&result.c_ptr_, c_ptr_,
                                          branch_name.c_str(),
                                          remote_url.c_str(), id.c_ptr()));
  return result;
}

annotated_commit
repository::create_annotated_commit(const std::string &revspec) const {
  annotated_commit result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_annotated_commit_from_revspec(&result.c_ptr_, c_ptr_,
                                        revspec.c_str()));
  return result;
}

annotated_commit repository::create_annotated_commit(const reference &ref) const {
  annotated_commit result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_annotated_commit_from_ref(&result.c_ptr_, c_ptr_, ref.c_ptr()));
  return result;
}

annotated_commit repository::lookup_annotated_commit(const oid &id) const {
  annotated_commit result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_annotated_commit_lookup(&result.c_ptr_, c_ptr_, id.c_ptr()));
  return result;
}

void repository::apply_diff(const diff &diff, apply::location location,
                            const apply::options &options) const {
  git_exception::throw_nonzero(
      git_apply(c_ptr_, const_cast<git_diff *>(diff.c_ptr()),
                static_cast<git_apply_location_t>(location), options.c_ptr()));
}

cppgit2::index repository::apply_diff(const tree &preimage, const diff &diff,
                                      const apply::options &options) const {
  git_index *result; // the postimage of the application
  git_exception::throw_nonzero(
      git_apply_to_tree(&result, c_ptr_,
                        const_cast<git_tree *>(preimage.c_ptr()),
                        const_cast<git_diff *>(diff.c_ptr()), options.c_ptr()));
  return cppgit2::index(result, ownership::user);
}

void repository::add_attributes_macro(const std::string &name,
                                      const std::string &values) const {
  git_exception::throw_nonzero(
      git_attr_add_macro(c_ptr_, name.c_str(), values.c_str()));
}

void repository::flush_attributes_cache() const {
  git_exception::throw_nonzero(
      git_attr_cache_flush(c_ptr_));
}

void repository::for_each_attribute(
    attribute::flag flags, const std::string &path,
    std::function<void(const std::string &, const std::string &)> visitor) const {

  struct visitor_wrapper {
    std::function<void(const std::string &, const std::string &)> fn;
  };

  visitor_wrapper wrapper;
  wrapper.fn = visitor;

  auto callback_c = [](const char *name, const char *value, void *payload) {
    auto wrapper = reinterpret_cast<visitor_wrapper *>(payload);
    wrapper->fn(name, value);
    return 0;
  };

  git_exception::throw_nonzero(
      git_attr_foreach(c_ptr_, static_cast<uint32_t>(flags), path.c_str(),
                       callback_c, (void *)(&wrapper)));
}

std::string repository::lookup_attribute(attribute::flag flags,
                                         const std::string &path,
                                         const std::string &name) const {
  const char *result;
  git_exception::throw_nonzero(
      git_attr_get(&result, c_ptr_, static_cast<uint32_t>(flags), path.c_str(),
                   name.c_str()));
  if (result)
    return std::string(result);
  else
    return "";
}

std::vector<std::string>
repository::lookup_multiple_attributes(attribute::flag flags,
                                       const std::string &path,
                                       const std::vector<std::string> &names) const {
  const char **values = (const char **)malloc(names.size());
  // const char *values[names.size()]; // TODO: Fix this

  std::vector<const char *> names_c;
  for (auto &name : names)
    names_c.push_back(name.c_str());

  git_exception::throw_nonzero(
      git_attr_get_many(values, c_ptr_, static_cast<uint32_t>(flags),
                        path.c_str(), names.size(), names_c.data()));

  std::vector<std::string> result;
  for (size_t i = 0; i < names.size(); ++i) {
    if (values[i]) {
      result.push_back(values[i]);
    } else {
      result.push_back("");
    }
  }
  free(values);
  return result;
}

blame repository::blame_file(const std::string &path,
                             blame::options options) const {
  blame result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_blame_file(&result.c_ptr_, c_ptr_, path.c_str(), options.c_ptr_));
  return result;
}

oid repository::create_blob_from_buffer(const std::string &buffer) const {
  oid result;
  git_exception::throw_nonzero(
      git_blob_create_frombuffer(result.c_ptr(), c_ptr_, buffer.c_str(),
                                 buffer.size()));
  return result;
}

oid repository::create_blob_from_disk(const std::string &path) const {
  oid result;
  git_exception::throw_nonzero(
      git_blob_create_fromdisk(result.c_ptr(), c_ptr_, path.c_str()));
  return result;
}

oid repository::create_blob_from_workdir(const std::string &relative_path) const {
  oid result;
  git_exception::throw_nonzero(
      git_blob_create_fromworkdir(result.c_ptr(), c_ptr_,
                                  relative_path.c_str()));
  return result;
}

blob repository::lookup_blob(const oid &id) const {
  blob result;
  git_exception::throw_nonzero(
      git_blob_lookup(&result.c_ptr_, c_ptr_, id.c_ptr()));
  return result;
}

blob repository::lookup_blob(const oid &id, size_t len) const {
  blob result;
  git_exception::throw_nonzero(
      git_blob_lookup_prefix(&result.c_ptr_, c_ptr_, id.c_ptr(), len));
  return result;
}

reference repository::create_branch(const std::string &branch_name,
                                    const commit &target, bool force) const {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_branch_create(&result.c_ptr_, c_ptr_, branch_name.c_str(),
                        target.c_ptr(), force));
  return result;
}

reference repository::create_branch(const std::string &branch_name,
                                    const annotated_commit &commit,
                                    bool force) const {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_branch_create_from_annotated(
          &result.c_ptr_, c_ptr_, branch_name.c_str(), commit.c_ptr(), force));
  return result;
}

void repository::delete_branch(const reference &ref) const {
  git_exception::throw_nonzero(
      git_branch_delete(ref.c_ptr_));
}

void repository::delete_branch(const std::string &branch_name,
                               branch::branch_type branch_type) const {
  delete_branch(lookup_branch(branch_name, branch_type));
}

bool repository::is_branch_checked_out(const reference &ref) const {
  return git_exception::throw_nonbool(
    git_branch_is_checked_out(ref.c_ptr()));
}

bool repository::is_branch_checked_out(const std::string &branch_name,
                                       branch::branch_type branch_type) const {
  return is_branch_checked_out(lookup_branch(branch_name, branch_type));
}

bool repository::is_head_pointing_to_branch(const reference &ref) const {
  // 1 if HEAD points at the branch, 0 if it isn't, or a negative value as an
  // error code.
  return git_exception::throw_nonbool(
    git_branch_is_head(ref.c_ptr()));
}

bool repository::is_head_pointing_to_branch(const std::string &branch_name,
                                            branch::branch_type branch_type) const {
  return is_head_pointing_to_branch(lookup_branch(branch_name, branch_type));
}

reference repository::rename_branch(const reference &ref,
                                    const std::string &new_branch_name,
                                    bool force) const {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_branch_move(&result.c_ptr_, ref.c_ptr_, new_branch_name.c_str(),
                      force));
  return result;
}

reference repository::rename_branch(const std::string &branch_name,
                                    const std::string &new_branch_name,
                                    bool force,
                                    branch::branch_type branch_type) const {
  auto ref = lookup_branch(branch_name, branch_type);
  return rename_branch(ref, new_branch_name, force);
}

std::string repository::branch_name(const reference &branch) const {
  const char *name;
  git_exception::throw_nonzero(
      git_branch_name(&name, branch.c_ptr()));
  if (name)
    return std::string(name);
  else
    return "";
}

std::string repository::branch_remote_name(const std::string &refname) const {
  git_buf buf = GIT_BUF_INIT;
  git_exception::throw_nonzero(
      git_branch_remote_name(&buf, c_ptr_, refname.c_str()));
  return data_buffer(&buf).to_string();
}

void repository::set_branch_upstream(const reference &ref,
                                     const std::string &upstream_name) const {
  git_exception::throw_nonzero(
      git_branch_set_upstream(ref.c_ptr_, upstream_name.c_str()));
}

void repository::set_branch_upstream(const std::string &branch_name,
                                     const std::string &upstream_name) const {
  auto ref = lookup_branch(branch_name, branch::branch_type::local);
  return set_branch_upstream(ref, upstream_name);
}

void repository::unset_branch_upstream(const reference &ref) const {
  git_exception::throw_nonzero(
      git_branch_set_upstream(ref.c_ptr_, nullptr));
}

void repository::unset_branch_upstream(const std::string &branch_name) const {
  auto ref = lookup_branch(branch_name, branch::branch_type::local);
  return unset_branch_upstream(ref);
}

reference repository::branch_upstream(const reference &local_branch) const {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_branch_upstream(&result.c_ptr_, local_branch.c_ptr()));
  return result;
}

reference repository::branch_upstream(const std::string &local_branch_name) const {
  auto ref = lookup_branch(local_branch_name, branch::branch_type::local);
  return branch_upstream(ref);
}

std::string repository::branch_upstream_name(const std::string &refname) const {
  git_buf buf = GIT_BUF_INIT;
  git_exception::throw_nonzero(
      git_branch_upstream_name(&buf, c_ptr_, refname.c_str()));
  return data_buffer(&buf).to_string();
}

std::string
repository::branch_upstream_remote(const std::string &refname) const {
  git_buf buf = GIT_BUF_INIT;
  git_exception::throw_nonzero(
      git_branch_upstream_remote(&buf, c_ptr_, refname.c_str()));
  return data_buffer(&buf).to_string();
}

reference repository::lookup_branch(const std::string &branch_name,
                                    branch::branch_type branch_type) const {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_branch_lookup(&result.c_ptr_, c_ptr_, branch_name.c_str(),
                        static_cast<git_branch_t>(branch_type)));
  return result;
}

void repository::for_each_branch(std::function<void(const reference &)> visitor,
                                 branch::branch_type branch_type) const {
  git_branch_t branch_type_c = static_cast<git_branch_t>(branch_type);
  git_branch_iterator *iter;
  git_branch_iterator_new(&iter, c_ptr_, branch_type_c);
  git_reference *ref_c;
  int ret;
  while ((ret = git_branch_next(&ref_c, &branch_type_c, iter)) == 0) {
    reference payload(ref_c);
    visitor(payload);
  }
  git_branch_iterator_free(iter);
}

void repository::checkout_head(const checkout::options &options) const {
  // Note that this is not the correct mechanism used to switch branches; do not
  // change your HEAD and then call this method, that would leave you with
  // checkout conflicts since your working directory would then appear to be
  // dirty. Instead, checkout the target of the branch and then update HEAD
  // using repository.set_head to point to the branch you checked out.
  git_exception::throw_nonzero(
      git_checkout_head(c_ptr_, options.c_ptr())); // options may be NULL
}

void repository::checkout_index(const cppgit2::index &index,
                                const checkout::options &options) const {
  git_exception::throw_nonzero(
      git_checkout_index(c_ptr_, const_cast<git_index *>(index.c_ptr()),
                         options.c_ptr())); // index & options may be NULL
}

void repository::checkout_tree(const object &treeish,
                               const checkout::options &options) const {
  git_exception::throw_nonzero(
      git_checkout_tree(c_ptr_, treeish.c_ptr(), options.c_ptr()));
}

void repository::cherrypick_commit(const commit &commit,
                                   const cherrypick::options &options) const {
  git_exception::throw_nonzero(
      git_cherrypick(c_ptr_, commit.c_ptr_, options.c_ptr()));
}

cppgit2::index
repository::cherrypick_commit(const commit &cherrypick_commit,
                              const commit &our_commit, unsigned int mainline,
                              const merge::options &merge_options) const {
  cppgit2::index result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_cherrypick_commit(&result.c_ptr_, c_ptr_, cherrypick_commit.c_ptr_,
                            our_commit.c_ptr_, mainline, merge_options.c_ptr()));
  return result;
}

oid repository::create_commit(const std::string &update_ref,
                              const signature &author,
                              const signature &committer,
                              const std::string &message_encoding,
                              const std::string &message, const tree &tree,
                              const std::vector<commit> &parents) const {
  oid result;
  const char *update_ref_c = (update_ref == "") ? NULL : update_ref.c_str();
  const char *message_encoding_c =
      message_encoding == "" ? NULL : message_encoding.c_str();
  std::vector<const git_commit *> parents_c;
  for (auto &p : parents) {
    parents_c.push_back(p.c_ptr());
  }
  git_exception::throw_nonzero(
      git_commit_create(result.c_ptr(), c_ptr_, update_ref_c, author.c_ptr(),
                        committer.c_ptr(), message_encoding_c, message.c_str(),
                        tree.c_ptr(), parents.size(), parents_c.data()));
  return result;
}

data_buffer repository::create_commit(const signature &author,
                                      const signature &committer,
                                      const std::string &message_encoding,
                                      const std::string &message,
                                      const tree &tree,
                                      const std::vector<commit> &parents) const {
  git_buf buf = GIT_BUF_INIT;
  const char *message_encoding_c =
      message_encoding == "" ? NULL : message_encoding.c_str();
  std::vector<const git_commit *> parents_c;
  for (auto &p : parents) {
    parents_c.push_back(p.c_ptr());
  }
  git_exception::throw_nonzero(
      git_commit_create_buffer(&buf, c_ptr_, author.c_ptr(),
                               committer.c_ptr(), message_encoding_c,
                               message.c_str(), tree.c_ptr(), parents.size(),
                               parents_c.data()));
  return data_buffer(&buf);
}

oid repository::create_commit(const std::string &commit_content,
                              const std::string &signature,
                              const std::string &signature_field) const {
  oid result;
  const char *signature_c = signature == "" ? NULL : signature.c_str();
  const char *signature_field_c =
      signature_field == "" ? "" : signature_field.c_str();
  git_exception::throw_nonzero(
      git_commit_create_with_signature(result.c_ptr(), c_ptr_,
                                       commit_content.c_str(), signature_c,
                                       signature_field_c));
  return result;
}

std::pair<data_buffer, data_buffer>
repository::extract_signature_from_commit(oid id,
                                          const std::string &signature_field) const {
  git_buf sig, signed_data;
  git_exception::throw_nonzero(
      git_commit_extract_signature(&sig, &signed_data, c_ptr_,
                                   id.c_ptr(), signature_field.c_str()));
  return std::pair<data_buffer, data_buffer>{std::move(data_buffer(&sig)), std::move(data_buffer(&signed_data))};
}

commit repository::lookup_commit(const oid &id) const {
  commit result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_commit_lookup(&result.c_ptr_, c_ptr_, id.c_ptr()));
  return result;
}

commit repository::lookup_commit(const oid &id, size_t length) const {
  commit result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_commit_lookup_prefix(&result.c_ptr_, c_ptr_, id.c_ptr(), length));
  return result;
}

void repository::for_each_commit(std::function<void(const commit &id)> visitor,
                                 revision::sort sort_ordering) const {
  git_revwalk *iter;
  auto ret = git_revwalk_new(&iter, c_ptr_);
  git_revwalk_push_head(iter);

  if (ret != 0) {
    git_revwalk_free(iter);
    throw git_exception(ret);
  }

  git_revwalk_sorting(iter, static_cast<unsigned int>(sort_ordering));
  git_oid id_c;
  while ((ret = git_revwalk_next(&id_c, iter)) == 0) {
    oid id(&id_c);
    visitor(lookup_commit(id));
  }
  git_revwalk_free(iter);
}

void repository::for_each_commit(std::function<void(const commit &id)> visitor,
                                 const commit &start_from,
                                 revision::sort sort_ordering) const {
  git_revwalk *iter;
  auto ret = git_revwalk_new(&iter, c_ptr_);
  git_revwalk_push(iter, start_from.id().c_ptr());

  if (ret != 0) {
    git_revwalk_free(iter);
    throw git_exception(ret);
  }

  git_revwalk_sorting(iter, static_cast<unsigned int>(sort_ordering));
  git_oid id_c;
  while ((ret = git_revwalk_next(&id_c, iter)) == 0) {
    oid payload(&id_c);
    visitor(lookup_commit(payload));
  }
  git_revwalk_free(iter);
}

void repository::add_ondisk_config_file(const cppgit2::config &cfg,
                                        const std::string &path,
                                        config::priority_level level,
                                        bool force) const {
  git_exception::throw_nonzero(
      git_config_add_file_ondisk(cfg.c_ptr_, path.c_str(),
                                 static_cast<git_config_level_t>(level), c_ptr_,
                                 force));
}

data_buffer repository::create_diff_commit_as_email(
    const commit &commit, size_t patch_no, size_t total_patches,
    diff::format_email_flag flags, const diff::options &options) const {
  git_buf buf = GIT_BUF_INIT;
  git_exception::throw_nonzero(
      git_diff_commit_as_email(&buf, c_ptr_, commit.c_ptr_, patch_no,
                               total_patches, static_cast<uint32_t>(flags),
                               options.c_ptr()));
  return data_buffer(&buf);
}

diff repository::create_diff_index_to_index(const cppgit2::index &old_index,
                                            const cppgit2::index &new_index,
                                            const diff::options &options) const {
  diff result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_diff_index_to_index(&result.c_ptr_, c_ptr_, old_index.c_ptr_,
                              new_index.c_ptr_, options.c_ptr()));
  return result;
}

diff repository::create_diff_index_to_workdir(const cppgit2::index &index,
                                              const diff::options &options) const {
  diff result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_diff_index_to_workdir(&result.c_ptr_, c_ptr_, index.c_ptr_,
                                options.c_ptr()));
  return result;
}

diff repository::create_diff_tree_to_index(const tree &old_tree,
                                           const cppgit2::index &index,
                                           const diff::options &options) const {
  diff result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_diff_tree_to_index(&result.c_ptr_, c_ptr_, old_tree.c_ptr_,
                             index.c_ptr_, options.c_ptr()));
  return result;
}

diff repository::create_diff_tree_to_tree(const tree &old_tree,
                                          const tree &new_tree,
                                          const diff::options &options) const {
  diff result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_diff_tree_to_tree(&result.c_ptr_, c_ptr_, old_tree.c_ptr_,
                            new_tree.c_ptr_, options.c_ptr()));
  return result;
}

diff repository::create_diff_tree_to_workdir(const tree &old_tree,
                                             const diff::options &options) const {
  diff result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_diff_tree_to_workdir(&result.c_ptr_, c_ptr_, old_tree.c_ptr_,
                               options.c_ptr()));
  return result;
}

diff repository::create_diff_tree_to_workdir_with_index(
    const tree &old_tree, const diff::options &options) const {
  diff result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_diff_tree_to_workdir_with_index(&result.c_ptr_, c_ptr_,
                                          old_tree.c_ptr_, options.c_ptr()));
  return result;
}

std::pair<size_t, size_t>
repository::unique_commits_ahead_behind(const oid &local,
                                        const oid &upstream) const {
  size_t ahead, behind;
  git_exception::throw_nonzero(
      git_graph_ahead_behind(&ahead, &behind, c_ptr_, local.c_ptr(),
                             upstream.c_ptr()));
  return std::pair<size_t, size_t>{ahead, behind};
}

bool repository::is_descendant_of(const oid &commit,
                                  const oid &ancestor) const {
  return git_graph_descendant_of(c_ptr_, commit.c_ptr(), ancestor.c_ptr());
}

void repository::add_ignore_rules(const std::string &rules) const {
  git_exception::throw_nonzero(
      git_ignore_add_rule(c_ptr_, rules.c_str()));
}

void repository::clear_ignore_rules() const {
  git_exception::throw_nonzero(
      git_ignore_clear_internal_rules(c_ptr_));
}

bool repository::is_path_ignored(const std::string &path) const {
  int result;
  git_exception::throw_nonzero(
      git_ignore_path_is_ignored(&result, c_ptr_, path.c_str()));
  return result;
}

std::pair<merge::analysis_result, merge::preference>
repository::analyze_merge(const std::vector<annotated_commit> &their_heads) const {
  git_merge_analysis_t analysis_result;
  git_merge_preference_t preference;

  std::vector<git_annotated_commit *const *> their_heads_c;
  size_t num_commits = their_heads.size();
  for (size_t i = 0; i < num_commits; ++i) {
    their_heads_c.push_back(&their_heads[i].c_ptr_);
  }

  git_exception::throw_nonzero(
      git_merge_analysis(&analysis_result, &preference, c_ptr_,
                         (const git_annotated_commit **)their_heads_c.data(),
                         num_commits));

  return std::pair<merge::analysis_result, merge::preference>{
      static_cast<merge::analysis_result>(analysis_result),
      static_cast<merge::preference>(preference)};
}

std::pair<merge::analysis_result, merge::preference>
repository::analyze_merge(const reference &our_ref,
                          const std::vector<annotated_commit> &their_heads) const {
  git_merge_analysis_t analysis_result;
  git_merge_preference_t preference;

  std::vector<git_annotated_commit *const *> their_heads_c;
  size_t num_commits = their_heads.size();
  for (size_t i = 0; i < num_commits; ++i) {
    their_heads_c.push_back(&their_heads[i].c_ptr_);
  }

  git_exception::throw_nonzero(
      git_merge_analysis_for_ref(
          &analysis_result, &preference, c_ptr_, our_ref.c_ptr_,
          (const git_annotated_commit **)their_heads_c.data(),
          their_heads.size()));

  return std::pair<merge::analysis_result, merge::preference>{
      static_cast<merge::analysis_result>(analysis_result),
      static_cast<merge::preference>(preference)};
}

oid repository::find_merge_base(const oid &first_commit,
                                const oid &second_commit) const {
  oid result;
  git_exception::throw_nonzero(
      git_merge_base(result.c_ptr(), c_ptr_, first_commit.c_ptr(),
                     second_commit.c_ptr()));
  return result;
}

oid repository::find_merge_base(const std::vector<oid> &commits) const {
  oid result;

  std::vector<git_oid> commits_c;
  for (auto &c : commits)
    commits_c.push_back(c.c_struct_);

  git_exception::throw_nonzero(
      git_merge_base_many(result.c_ptr(), c_ptr_, commits.size(),
                          commits_c.data()));

  return result;
}

oid repository::find_merge_base_for_octopus_merge(
    const std::vector<oid> &commits) const {
  oid result;

  std::vector<git_oid> commits_c;
  for (auto &c : commits)
    commits_c.push_back(c.c_struct_);

  git_exception::throw_nonzero(
      git_merge_base_octopus(result.c_ptr(), c_ptr_, commits.size(),
                             commits_c.data()));

  return result;
}

std::vector<oid> repository::find_merge_bases(const oid &first_commit,
                                              const oid &second_commit) const {
  std::vector<oid> result{};
  git_oidarray result_c;
  git_exception::throw_nonzero(
      git_merge_bases(&result_c, c_ptr_, first_commit.c_ptr(),
                      second_commit.c_ptr()));

  for (size_t i = 0; i < result_c.count; ++i) {
    result.push_back(oid(&result_c.ids[i]));
  }
  return result;
}

std::vector<oid> repository::find_merge_bases(const std::vector<oid> &commits) const {
  std::vector<oid> result{};
  git_oidarray result_c;

  std::vector<git_oid> commits_c;
  for (auto &c : commits)
    commits_c.push_back(c.c_struct_);

  git_exception::throw_nonzero(
      git_merge_bases_many(&result_c, c_ptr_, commits.size(), commits_c.data()));

  for (size_t i = 0; i < result_c.count; ++i) {
    result.push_back(oid(&result_c.ids[i]));
  }
  return result;
}

void repository::merge_commits(const std::vector<annotated_commit> &their_heads,
                               const merge::options &merge_options,
                               const checkout::options &checkout_options) const {
  std::vector<git_annotated_commit *const *> their_heads_c;
  size_t num_commits = their_heads.size();
  for (size_t i = 0; i < num_commits; ++i) {
    their_heads_c.push_back(&their_heads[i].c_ptr_);
  }

  git_exception::throw_nonzero(
      git_merge(c_ptr_, (const git_annotated_commit **)their_heads_c.data(),
                their_heads.size(), merge_options.c_ptr(),
                checkout_options.c_ptr()));
}

cppgit2::index repository::merge_commits(const commit &our_commit,
                                         const commit &their_commit,
                                         const merge::options &merge_options) const {
  cppgit2::index result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_merge_commits(&result.c_ptr_, c_ptr_, our_commit.c_ptr_,
                        their_commit.c_ptr_, merge_options.c_ptr()));
  return result;
}

merge::file::result repository::merge_file_from_index(
    const index::entry &ancestor, const index::entry &ours,
    const index::entry &theirs, const merge::file::options &options) const {
  git_merge_file_result result;
  git_exception::throw_nonzero(
      git_merge_file_from_index(&result, c_ptr_, ancestor.c_ptr(), ours.c_ptr(),
                                theirs.c_ptr(), options.c_ptr()));
  return merge::file::result(&result);
}

cppgit2::index repository::merge_trees(const tree &ancestor_tree,
                                       const tree &our_tree,
                                       const tree &their_tree,
                                       const merge::options &options) const {
  cppgit2::index result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_merge_trees(&result.c_ptr_, c_ptr_, ancestor_tree.c_ptr_,
                      our_tree.c_ptr_, their_tree.c_ptr_, options.c_ptr()));
  return result;
}

oid repository::create_note(const std::string &notes_ref,
                            const signature &author, const signature &committer,
                            const oid &id, const std::string &note,
                            bool force) const {
  oid result;
  git_exception::throw_nonzero(
      git_note_create(result.c_ptr(), c_ptr_, notes_ref.c_str(), author.c_ptr(),
                      committer.c_ptr(), id.c_ptr(), note.c_str(), force));
  return result;
}

std::pair<oid, oid>
repository::create_note(const commit &parent, const signature &author,
                        const signature &committer, const oid &id,
                        const std::string &note, bool allow_note_override) const {
  oid notes_commit_out, notes_blob_out;
  git_exception::throw_nonzero(
      git_note_commit_create(notes_commit_out.c_ptr(), notes_blob_out.c_ptr(),
                             c_ptr_, parent.c_ptr_, author.c_ptr(),
                             committer.c_ptr(), id.c_ptr(), note.c_str(),
                             allow_note_override));
  return std::pair<oid, oid>{notes_commit_out, notes_blob_out};
}

note repository::read_note(const std::string &notes_ref, const oid &id) const {
  note result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_note_read(&result.c_ptr_, c_ptr_, notes_ref.c_str(), id.c_ptr()));
  return result;
}

note repository::read_note(const commit &notes_commit, const oid &id) const {
  note result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_note_commit_read(&result.c_ptr_, c_ptr_, notes_commit.c_ptr_,
                           id.c_ptr()));
  return result;
}

void repository::remove_note(const std::string &notes_ref,
                             const signature &author,
                             const signature &committer, const oid &id) const {
  git_exception::throw_nonzero(
      git_note_remove(c_ptr_, notes_ref.c_str(), author.c_ptr(),
                      committer.c_ptr(), id.c_ptr()));
}

oid repository::remove_note(const commit &notes_commit, const signature &author,
                            const signature &committer, const oid &id) const {
  oid result;
  git_exception::throw_nonzero(
      git_note_commit_remove(result.c_ptr(), c_ptr_, notes_commit.c_ptr_,
                             author.c_ptr(), committer.c_ptr(), id.c_ptr()));
  return result;
}

data_buffer repository::detault_notes_reference() const {
  git_buf buf = GIT_BUF_INIT;
  git_exception::throw_nonzero(
      git_note_default_ref(&buf, c_ptr_));
  return data_buffer(&buf);
}

void repository::for_each_note(
    const std::string &notes_ref,
    std::function<void(const oid &, const oid &)> visitor) const {
  struct visitor_wrapper {
    std::function<void(const oid &, const oid &)> fn;
  };

  visitor_wrapper wrapper;
  wrapper.fn = visitor;

  auto callback_c = [](const git_oid *blob_id,
                       const git_oid *annotated_object_id, void *payload) {
    auto wrapper = reinterpret_cast<visitor_wrapper *>(payload);
    wrapper->fn(oid(blob_id), oid(annotated_object_id));
    return 0;
  };

  git_exception::throw_nonzero(
      git_note_foreach(c_ptr_, notes_ref.c_str(), callback_c,
                       (void *)(&wrapper)));
}

object repository::lookup_object(const oid &id,
                                 object::object_type type) const {
  object result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_object_lookup(&result.c_ptr_, c_ptr_, id.c_ptr(),
                      static_cast<git_object_t>(type)));
  return result;
}

object repository::lookup_object(const oid &id, size_t length,
                                 object::object_type type) const {
  object result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_object_lookup_prefix(&result.c_ptr_, c_ptr_, id.c_ptr(), length,
                               static_cast<git_object_t>(type)));
  return result;
}

object repository::lookup_object(const object &treeish, const std::string &path,
                                 object::object_type type) const {
  object result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_object_lookup_bypath(&result.c_ptr_, treeish.c_ptr_, path.c_str(),
                               static_cast<git_object_t>(type)));
  return result;
}

pack_builder repository::initialize_pack_builder() const {
  pack_builder result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_packbuilder_new(&result.c_ptr_, c_ptr_));
  return result;
}

rebase repository::init_rebase(const annotated_commit &branch,
                               const annotated_commit &upstream,
                               const annotated_commit &onto,
                               const rebase::options &options) const {
  rebase result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_rebase_init(&result.c_ptr_, c_ptr_, branch.c_ptr(), upstream.c_ptr(),
                      onto.c_ptr(), options.c_ptr()));
  return result;
}

rebase repository::open_rebase(const rebase::options &options) const {
  rebase result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_rebase_open(&result.c_ptr_, c_ptr_, options.c_ptr()));
  return result;
}

cppgit2::refdb repository::create_refdb() const {
  cppgit2::refdb result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_refdb_new(&result.c_ptr_, c_ptr_));
  return result;
}

cppgit2::refdb repository::open_refdb() const {
  cppgit2::refdb result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_refdb_open(&result.c_ptr_, c_ptr_));
  return result;
}

reference repository::create_reference(const std::string &name, const oid &id,
                                       bool force,
                                       const std::string &log_message) const {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_reference_create(&result.c_ptr_, c_ptr_, name.c_str(), id.c_ptr(),
                           force, log_message.c_str()));
  return result;
}

reference repository::create_reference(const std::string &name, const oid &id,
                                       bool force, const oid &current_id,
                                       const std::string &log_message) const {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_reference_create_matching(&result.c_ptr_, c_ptr_, name.c_str(),
                                    id.c_ptr(), force, current_id.c_ptr(),
                                    log_message.c_str()));
  return result;
}

void repository::delete_reference(const std::string &refname) const {
  git_exception::throw_nonzero(
      git_reference_remove(c_ptr_, refname.c_str()));
}

reference
repository::lookup_reference_by_dwim(const std::string &shorthand_name) const {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_reference_dwim(&result.c_ptr_, c_ptr_, shorthand_name.c_str()));
  return result;
}

void repository::ensure_reflog_for_reference(const std::string &refname) const {
  git_exception::throw_nonzero(
      git_reference_ensure_log(c_ptr_, refname.c_str()));
}

bool repository::reference_has_reflog(const std::string &refname) const {
  return git_reference_has_log(c_ptr_, refname.c_str());
}

strarray repository::reference_list() const {
  strarray result;
  git_exception::throw_nonzero(
      git_reference_list(&result.c_struct_, c_ptr_));
  return result;
}

reference repository::lookup_reference(const std::string &refname) const {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_reference_lookup(&result.c_ptr_, c_ptr_, refname.c_str()));
  return result;
}

oid repository::reference_name_to_id(const std::string &refname) const {
  oid result;
  git_exception::throw_nonzero(
      git_reference_name_to_id(result.c_ptr(), c_ptr_, refname.c_str()));
  return result;
}

reference
repository::create_symbolic_reference(const std::string &name,
                                      const std::string &target, bool force,
                                      const std::string &log_message) const {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_reference_symbolic_create(&result.c_ptr_, c_ptr_, name.c_str(),
                                    target.c_str(), force, log_message.c_str()));
  return result;
}

reference repository::create_symbolic_reference(
    const std::string &name, const std::string &target, bool force,
    const std::string &current_value, const std::string &log_message) const {
  reference result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_reference_symbolic_create_matching(
          &result.c_ptr_, c_ptr_, name.c_str(), target.c_str(), force,
          current_value.c_str(), log_message.c_str()));
  return result;
}

void repository::for_each_reference(
    std::function<void(const reference &)> visitor) const {
  git_reference_iterator *iter;
  git_reference_iterator_new(&iter, c_ptr_);
  git_reference *ref_c;
  int ret;
  while ((ret = git_reference_next(&ref_c, iter)) == 0) {
    reference payload(ref_c);
    visitor(payload);
  }
  git_reference_iterator_free(iter);
}

void repository::for_each_reference_name(
    std::function<void(const std::string &)> visitor) const {
  git_reference_iterator *iter;
  git_reference_iterator_new(&iter, c_ptr_);
  const char *refname_c;
  int ret;
  while ((ret = git_reference_next_name(&refname_c, iter)) == 0) {
    std::string payload{""};
    if (refname_c)
      payload = std::string(refname_c);
    visitor(payload);
  }
  git_reference_iterator_free(iter);
}

void repository::for_each_reference_glob(
    const std::string &glob, std::function<void(const std::string &)> visitor) const {
  git_reference_iterator *iter;
  git_reference_iterator_glob_new(&iter, c_ptr_, glob.c_str());
  const char *refname_c;
  int ret;
  while ((ret = git_reference_next_name(&refname_c, iter)) == 0) {
    std::string payload{""};
    if (refname_c)
      payload = std::string(refname_c);
    visitor(payload);
  }
  git_reference_iterator_free(iter);
}

void repository::delete_reflog(const std::string &name) const {
  git_exception::throw_nonzero(
      git_reflog_delete(c_ptr_, name.c_str()));
}

reflog repository::read_reflog(const std::string &name) const {
  reflog result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_reflog_read(&result.c_ptr_, c_ptr_, name.c_str()));
  return result;
}

void repository::rename_reflog(const std::string &old_name,
                               const std::string &name) const {
  git_exception::throw_nonzero(
      git_reflog_rename(c_ptr_, old_name.c_str(), name.c_str()));
}

void repository::reset(const object &target, reset::reset_type reset_type,
                       const checkout::options &options) const {
  git_exception::throw_nonzero(
      git_reset(c_ptr_, target.c_ptr(), static_cast<git_reset_t>(reset_type),
                options.c_ptr()));
}

void repository::reset_default(const object &target,
                               const std::vector<std::string> &pathspecs) const {
  git_exception::throw_nonzero(
      git_reset_default(c_ptr_, target.c_ptr(), strarray(pathspecs).c_ptr()));
}

void repository::reset(const annotated_commit &target,
                       reset::reset_type reset_type,
                       const checkout::options &options) const {
  git_exception::throw_nonzero(
      git_reset_from_annotated(c_ptr_, target.c_ptr(),
                               static_cast<git_reset_t>(reset_type),
                               options.c_ptr()));
}

void repository::add_fetch_refspec_to_remote(const std::string &remote,
                                             const std::string &refspec) const {
  git_exception::throw_nonzero(
      git_remote_add_fetch(c_ptr_, remote.c_str(), refspec.c_str()));
}

void repository::add_push_refspec_to_remote(const std::string &remote,
                                            const std::string &refspec) const {
  git_exception::throw_nonzero(
      git_remote_add_push(c_ptr_, remote.c_str(), refspec.c_str()));
}

remote repository::create_remote(const std::string &name,
                                 const std::string &url) const {
  remote result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_remote_create(&result.c_ptr_, c_ptr_, name.c_str(), url.c_str()));
  return result;
}

remote repository::create_anonymous_remote(const std::string &url) const {
  remote result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_remote_create_anonymous(&result.c_ptr_, c_ptr_, url.c_str()));
  return result;
}

remote repository::create_remote(const std::string &name,
                                 const std::string &url,
                                 const std::string &fetch_refspec) const {
  remote result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_remote_create_with_fetchspec(&result.c_ptr_, c_ptr_, name.c_str(),
                                       url.c_str(), fetch_refspec.c_str()));
  return result;
}

void repository::delete_remote(const std::string &name) const {
  git_exception::throw_nonzero(
      git_remote_delete(c_ptr_, name.c_str()));
}

strarray repository::remote_list() const {
  strarray result;
  git_exception::throw_nonzero(
      git_remote_list(&result.c_struct_, c_ptr_));
  return result;
}

remote repository::lookup_remote(const std::string &name) const {
  remote result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_remote_lookup(&result.c_ptr_, c_ptr_, name.c_str()));
  return result;
}

strarray repository::rename_remote(const std::string &name,
                                   const std::string &new_name) const {
  strarray result;
  git_exception::throw_nonzero(
      git_remote_rename(&result.c_struct_, c_ptr_, name.c_str(),
                        new_name.c_str()));
  return result;
}

void repository::set_remote_autotag(const std::string &remote,
                                    fetch::options::autotag option) const {
  git_exception::throw_nonzero(
      git_remote_set_autotag(c_ptr_, remote.c_str(),
                             static_cast<git_remote_autotag_option_t>(option)));
}

void repository::set_remote_push_url(const std::string &remote,
                                     const std::string &url) const {
  git_exception::throw_nonzero(
      git_remote_set_pushurl(c_ptr_, remote.c_str(), url.c_str()));
}

void repository::set_remote_url(const std::string &remote,
                                const std::string &url) const {
  git_exception::throw_nonzero(
      git_remote_set_url(c_ptr_, remote.c_str(), url.c_str()));
}

void repository::revert_commit(const commit &commit,
                               const revert::options &options) const {
  git_exception::throw_nonzero(
      git_revert(c_ptr_, commit.c_ptr_, options.c_ptr()));
}

cppgit2::index repository::revert_commit(const commit &revert_commit,
                                         const commit &our_commit,
                                         unsigned int mainline,
                                         const merge::options &options) const {
  cppgit2::index result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_revert_commit(&result.c_ptr_, c_ptr_, revert_commit.c_ptr_,
                        our_commit.c_ptr_, mainline, options.c_ptr()));
  return result;
}

revspec repository::revparse(const std::string &spec) const {
  revspec result;
  git_exception::throw_nonzero(
      git_revparse(result.c_ptr_, c_ptr_, spec.c_str()));
  result.from_ = object(result.c_ptr_->from, ownership::user);
  result.to_ = object(result.c_ptr_->to, ownership::user);
  return result;
}

std::pair<object, reference>
repository::revparse_to_object_and_reference(const std::string &spec) const {
  object object_out(nullptr, ownership::user);
  reference reference_out(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_revparse_ext(&object_out.c_ptr_, &reference_out.c_ptr_, c_ptr_,
                       spec.c_str()));
  return std::pair<object, reference>{object_out, reference_out};
}

object repository::revparse_to_object(const std::string &spec) const {
  object result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_revparse_single(&result.c_ptr_, c_ptr_, spec.c_str()));
  return result;
}

revwalk repository::create_revwalk() const {
  revwalk result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_revwalk_new(&result.c_ptr_, c_ptr_));
  return result;
}

signature repository::default_signature() const {
  signature result;
  git_exception::throw_nonzero(
      git_signature_default(&result.c_ptr_, c_ptr_));
  return result;
}

void repository::apply_stash(size_t index,
                             const stash::apply::options &options) const {
  git_exception::throw_nonzero(
      git_stash_apply(c_ptr_, index, options.c_ptr()));
}

void repository::drop_stash(size_t index) const {
  git_exception::throw_nonzero(
      git_stash_drop(c_ptr_, index));
}

void repository::for_each_stash(
    std::function<void(size_t, const std::string &, const oid &)> visitor)
    const {
  struct visitor_wrapper {
    std::function<void(size_t, const std::string &, const oid &)> fn;
  };

  visitor_wrapper wrapper;
  wrapper.fn = visitor;

  auto callback_c = [](size_t index, const char *message,
                       const git_oid *stash_id, void *payload) {
    auto wrapper = reinterpret_cast<visitor_wrapper *>(payload);
    wrapper->fn(index, message, oid(stash_id));
    return 0;
  };

  git_exception::throw_nonzero(
      git_stash_foreach(c_ptr_, callback_c, (void *)(&wrapper)));
}

void repository::pop_stash(size_t index, const stash::apply::options &options) const {
  git_exception::throw_nonzero(
      git_stash_pop(c_ptr_, index, options.c_ptr()));
}

oid repository::save_stash(const signature &stasher, const std::string &message,
                           stash::apply::flag flags) const {
  oid result;
  git_exception::throw_nonzero(
      git_stash_save(result.c_ptr(), c_ptr_, stasher.c_ptr(), message.c_str(),
                     static_cast<uint32_t>(flags)));
  return result;
}

status::status_type repository::status_file(const std::string &path) const {
  unsigned int result;
  git_exception::throw_nonzero(
      git_status_file(&result, c_ptr_, path.c_str()));
  return static_cast<status::status_type>(result);
}

void repository::for_each_status(
    std::function<void(const std::string &, status::status_type)> visitor) const {
  struct visitor_wrapper {
    std::function<void(const std::string &, status::status_type)> fn;
  };

  visitor_wrapper wrapper;
  wrapper.fn = visitor;

  auto callback_c = [](const char *path, unsigned int status_flags,
                       void *payload) {
    auto wrapper = reinterpret_cast<visitor_wrapper *>(payload);
    wrapper->fn(path, static_cast<status::status_type>(status_flags));
    return 0;
  };

  git_exception::throw_nonzero(
      git_status_foreach(c_ptr_, callback_c, (void *)(&wrapper)));
}

void repository::for_each_status(
    const status::options &options,
    std::function<void(const std::string &, status::status_type)> visitor) const {
  struct visitor_wrapper {
    std::function<void(const std::string &, status::status_type)> fn;
  };

  visitor_wrapper wrapper;
  wrapper.fn = visitor;

  auto callback_c = [](const char *path, unsigned int status_flags,
                       void *payload) {
    auto wrapper = reinterpret_cast<visitor_wrapper *>(payload);
    wrapper->fn(path, static_cast<status::status_type>(status_flags));
    return 0;
  };

  git_exception::throw_nonzero(
      git_status_foreach_ext(c_ptr_, options.c_ptr(), callback_c,
                           (void *)(&wrapper)));
}

status::list repository::status_list(const status::options &options) const {
  status::list result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_status_list_new(&result.c_ptr_, c_ptr_, options.c_ptr()));
  return result;
}

bool repository::should_ignore(const std::string &path) const {
  int result;
  git_exception::throw_nonzero(
      git_status_should_ignore(&result, c_ptr_, path.c_str()));
  return result;
}

submodule repository::setup_submodule(const std::string &url,
                                      const std::string &path,
                                      bool use_gitlink) const {
  submodule result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_submodule_add_setup(&result.c_ptr_, c_ptr_, url.c_str(), path.c_str(),
                              use_gitlink));
  return result;
}

void repository::for_each_submodule(
    std::function<void(const submodule &, const std::string &)> visitor) const {
  // Wrap user-provided visitor funciton in a struct
  struct visitor_wrapper {
    std::function<void(const submodule &, const std::string &)> fn;
  };

  visitor_wrapper wrapper;
  wrapper.fn = visitor;

  // Pass wrapper visitor as the payload variable to the c callback

  auto visitor_c = [](git_submodule *sm, const char *name, void *payload) {
    auto wrapped = reinterpret_cast<visitor_wrapper *>(payload);

    submodule submodule_arg = submodule(sm);
    std::string name_arg = std::string(name);

    wrapped->fn(submodule_arg, name_arg); // call the wrapper visitor function
    return 0;
  };

  git_exception::throw_nonzero(
      git_submodule_foreach(c_ptr_, visitor_c, (void *)(&wrapper)));
}

submodule repository::lookup_submodule(const std::string &name) const {
  submodule result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_submodule_lookup(&result.c_ptr_, c_ptr_, name.c_str()));
  return result;
}

data_buffer repository::resolve_submodule_url(const std::string &url) const {
  git_buf buf = GIT_BUF_INIT;
  git_exception::throw_nonzero(
      git_submodule_resolve_url(&buf, c_ptr_, url.c_str()));
  return data_buffer(&buf);
}

void repository::set_submodule_branch(const std::string &submodule_name,
                                      const std::string &branch_name) const {
  git_exception::throw_nonzero(
      git_submodule_set_branch(c_ptr_, submodule_name.c_str(),
                             branch_name.c_str()));
}

void repository::set_submodule_fetch_recurse_option(
    const std::string &submodule_name,
    submodule::recurse fetch_recurse_submodules) const {
  git_exception::throw_nonzero(
      git_submodule_set_fetch_recurse_submodules(
          c_ptr_, submodule_name.c_str(),
          static_cast<git_submodule_recurse_t>(fetch_recurse_submodules)));
}

void repository::set_submodule_ignore_option(const std::string &submodule_name,
                                             submodule::ignore ignore) const {
  git_exception::throw_nonzero(
      git_submodule_set_ignore(c_ptr_, submodule_name.c_str(),
                               static_cast<git_submodule_ignore_t>(ignore)));
}

void repository::set_submodule_update_option(
    const std::string &submodule_name, submodule::update_strategy update) const {
  git_exception::throw_nonzero(
      git_submodule_set_update(c_ptr_, submodule_name.c_str(),
                               static_cast<git_submodule_update_t>(update)));
}

void repository::set_submodule_url(const std::string &submodule_name,
                                   const std::string &submodule_url) const {
  git_exception::throw_nonzero(
      git_submodule_set_url(c_ptr_, submodule_name.c_str(),
                            submodule_url.c_str()));
}

submodule::status repository::submodule_status(const std::string &name,
                                               submodule::ignore ignore) const {
  unsigned int result;
  git_exception::throw_nonzero(
      git_submodule_status(&result, c_ptr_, name.c_str(),
                           static_cast<git_submodule_ignore_t>(ignore)));
  return static_cast<submodule::status>(result);
}

oid repository::create_tag_annotation(const std::string &tag_name,
                                      const object &target,
                                      const signature &tagger,
                                      const std::string &message) const {
  oid result;
  git_exception::throw_nonzero(
      git_tag_annotation_create(result.c_ptr(), c_ptr_, tag_name.c_str(),
                                target.c_ptr(), tagger.c_ptr(),
                                message.c_str()));
  return result;
}

oid repository::create_tag(const std::string &tag_name, const object &target,
                           const signature &tagger, const std::string &message,
                           bool force) const {
  oid result;
  git_exception::throw_nonzero(
      git_tag_create(result.c_ptr(), c_ptr_, tag_name.c_str(), target.c_ptr(),
                     tagger.c_ptr(), message.c_str(), force));
  return result;
}

oid repository::create_tag(const std::string &buffer, bool force) const {
  oid result;
  git_exception::throw_nonzero(
      git_tag_create_frombuffer(result.c_ptr(), c_ptr_, buffer.c_str(), force));
  return result;
}

oid repository::create_lightweight_tag(const std::string &tag_name,
                                       const object &target, bool force) const {
  oid result;
  git_exception::throw_nonzero(
      git_tag_create_lightweight(result.c_ptr(), c_ptr_, tag_name.c_str(),
                               target.c_ptr(), force));
  return result;
}

void repository::delete_tag(const std::string &tag_name) const {
  git_exception::throw_nonzero(
      git_tag_delete(c_ptr_, tag_name.c_str()));
}

void repository::for_each_tag(
    std::function<void(const std::string &, const oid &)> visitor) const {
  struct visitor_wrapper {
    std::function<void(const std::string &, const oid &)> fn;
  };

  visitor_wrapper wrapper;
  wrapper.fn = visitor;

  auto callback_c = [](const char *name, git_oid *oid_c, void *payload) {
    auto wrapper = reinterpret_cast<visitor_wrapper *>(payload);
    wrapper->fn(name, oid(oid_c));
    return 0;
  };

  git_exception::throw_nonzero(
      git_tag_foreach(c_ptr_, callback_c, (void *)(&wrapper)));
}

strarray repository::tags() const {
  strarray result;
  git_exception::throw_nonzero(
      git_tag_list(&result.c_struct_, c_ptr_));
  return result;
}

strarray repository::tags_that_match(const std::string &pattern) const {
  strarray result;
  git_exception::throw_nonzero(
      git_tag_list_match(&result.c_struct_, pattern.c_str(), c_ptr_));
  return result;
}

tag repository::lookup_tag(const oid &id) const {
  tag result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_tag_lookup(&result.c_ptr_, c_ptr_, id.c_ptr()));
  return result;
}

tag repository::lookup_tag(const oid &id, size_t length) const {
  tag result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_tag_lookup_prefix(&result.c_ptr_, c_ptr_, id.c_ptr(), length));
  return result;
}

transaction repository::create_transaction() const {
  transaction result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_transaction_new(&result.c_ptr_, c_ptr_));
  return result;
}

object repository::tree_entry_to_object(const tree::entry &entry) const {
  object result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_tree_entry_to_object(&result.c_ptr_, c_ptr_, entry.c_ptr()));
  return result;
}

tree repository::lookup_tree(const oid &id) const {
  tree result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_tree_lookup(&result.c_ptr_, c_ptr_, id.c_ptr()));
  return result;
}

tree repository::lookup_tree(const oid &id, size_t length) const {
  tree result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_tree_lookup_prefix(&result.c_ptr_, c_ptr_, id.c_ptr(), length));
  return result;
}

oid repository::create_updated_tree(const tree &baseline,
                                    std::vector<tree::update> updates) const {
  oid result;
  git_tree_update *updates_c =
      (git_tree_update *)malloc(updates.size() * sizeof(git_tree_update));
  for (size_t i = 0; i < updates.size(); ++i)
    updates_c[i] = updates[i].c_struct_;

  git_exception::throw_nonzero(
      git_tree_create_updated(result.c_ptr(), c_ptr_, baseline.c_ptr_,
                            updates.size(), updates_c));
  free(updates_c);
  return result;
}

worktree repository::add_worktree(const std::string &name,
                                  const std::string &path,
                                  const worktree::add_options &options) const {
  worktree result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_worktree_add(&result.c_ptr_, c_ptr_, name.c_str(), path.c_str(),
                       options.c_ptr()));
  return result;
}

strarray repository::list_worktrees() const {
  strarray result;
  git_exception::throw_nonzero(
      git_worktree_list(&result.c_struct_, c_ptr_));
  return result;
}

worktree repository::lookup_worktree(const std::string &name) const {
  worktree result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_worktree_lookup(&result.c_ptr_, c_ptr_, name.c_str()));
  return result;
}

worktree repository::open_worktree() const {
  worktree result(nullptr, ownership::user);
  git_exception::throw_nonzero(
      git_worktree_open_from_repository(&result.c_ptr_, c_ptr_));
  return result;
}

} // namespace cppgit2
