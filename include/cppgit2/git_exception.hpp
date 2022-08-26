#pragma once
#include <exception>
#include <git2.h>
#include <string>

namespace cppgit2 {

class git_exception : public std::exception {
public:
  enum class error_class;
  enum class error_code;

  git_exception(int return_code = static_cast<int>(error_code::error)) {
    auto error = git_error_last();
    code_ = return_code <= 0 ? static_cast<error_code>(return_code) : error_code::error;
    message_ = error ? error->message : "unknown error";
    klass_ = error ? static_cast<error_class>(error->klass) : error_class::none;
  }
  git_exception(const char *message,
                error_class klass = error_class::none,
                error_code code = error_code::error)
  : message_(message),
    code_(code),
    klass_(klass)
  {}
  git_exception(const std::string &message,
                error_class klass = error_class::none,
                error_code code = error_code::error)
  : message_(message),
    code_(code),
    klass_(klass)
  {}
  virtual ~git_exception() throw() {}
  virtual const char *what() const throw() { return message_.c_str(); }
  error_code code() const { return code_; }
  error_class klass() const { return klass_; }

  static void clear() { git_error_clear(); }

  static int throw_nonzero(int return_code) {
    if (return_code == 0) {
      return return_code;
    } else {
      throw git_exception(return_code);
    }
  }
  static bool throw_nonbool(int return_code) {
    if (return_code == 1)
      return true;
    else if (return_code == 0)
      return false;
    else
      throw git_exception(return_code);
  }

  enum class error_code : int {
    ok             =   0, // No error
    error          =  -1, // Generic error
    notfound       =  -3, // Requested object could not be found
    exists         =  -4, // Object exists preventing operation
    ambiguous      =  -5, // More than one object matches
    bufs           =  -6, // Output buffer too short to hold data
    user           =  -7, // User callback custom failure
    barerepo       =  -8, // Operation not allowed on bare repository
    unbornbranch   =  -9, // HEAD refers to branch with no commits
    unmerged       = -10, // Merge in progress prevented operation
    nonfastforward = -11, // Reference was not fast-forwardable
    invalidspec    = -12, // Name/ref spec was not in a valid format
    conflict       = -13, // Checkout conflicts prevented operation
    locked         = -14, // Lock file prevented operation
    modified       = -15, // Reference value does not match expected
    auth           = -16, // Authentication error
    certificate    = -17, // Server certificate is invalid
    applied        = -18, // Patch/merge has already been applied
    peel           = -19, // The requested peel operation is not possible
    eof            = -20, // Unexpected EOF
    invalid        = -21, // Invalid operation or input
    uncommitted    = -22, // Uncommitted changes in index prevented operation
    directory      = -23, // The operation is not valid for a directory
    mergeconflict  = -24, // A merge conflict exists and cannot continue
  
    passthrough    = -30, // A user-configured callback refused to act
    iterover       = -31, // Signals end of iteration with iterator
    retry          = -32, // Internal only
    mismatch       = -33, // Hashsum mismatch in object
    indexdirty     = -34, // Unsaved changes in the index would be overwritten
    applyfail      = -35, // Patch application failed
    owner          = -36, // The object is not owned by the current user
  };

  enum class error_class : int {
    none           = GIT_ERROR_NONE,
    nomemory       = GIT_ERROR_NOMEMORY,
    os             = GIT_ERROR_OS,
    invalid        = GIT_ERROR_INVALID,
    reference      = GIT_ERROR_REFERENCE,
    zlib           = GIT_ERROR_ZLIB,
    repository     = GIT_ERROR_REPOSITORY,
    config         = GIT_ERROR_CONFIG,
    regex          = GIT_ERROR_REGEX,
    odb            = GIT_ERROR_ODB,
    index          = GIT_ERROR_INDEX,
    object         = GIT_ERROR_OBJECT,
    net            = GIT_ERROR_NET,
    tag            = GIT_ERROR_TAG,
    tree           = GIT_ERROR_TREE,
    indexer        = GIT_ERROR_INDEXER,
    ssl            = GIT_ERROR_SSL,
    submodule      = GIT_ERROR_SUBMODULE,
    thread         = GIT_ERROR_THREAD,
    stash          = GIT_ERROR_STASH,
    checkout       = GIT_ERROR_CHECKOUT,
    fetchhead      = GIT_ERROR_FETCHHEAD,
    merge          = GIT_ERROR_MERGE,
    ssh            = GIT_ERROR_SSH,
    filter         = GIT_ERROR_FILTER,
    revert         = GIT_ERROR_REVERT,
    callback       = GIT_ERROR_CALLBACK,
    cherrypick     = GIT_ERROR_CHERRYPICK,
    describe       = GIT_ERROR_DESCRIBE,
    rebase         = GIT_ERROR_REBASE,
    filesystem     = GIT_ERROR_FILESYSTEM,
    patch          = GIT_ERROR_PATCH,
    worktree       = GIT_ERROR_WORKTREE,
    //sha            = GIT_ERROR_SHA,
    http           = GIT_ERROR_HTTP,
    internal       = GIT_ERROR_INTERNAL,
  };

protected:
  std::string message_;
  error_code code_;
  error_class klass_;
};

} // namespace cppgit2
