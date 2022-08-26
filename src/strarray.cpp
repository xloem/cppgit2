#include <cppgit2/strarray.hpp>
#include <cstdlib>
#include <cstring>

namespace cppgit2 {

strarray::strarray() {
  c_struct_.count = 0;
  c_struct_.strings = nullptr;
}

strarray::strarray(const std::vector<std::string> &strings) {
  auto size = strings.size();
  c_struct_.count = size;
  c_struct_.strings = (char **)malloc(size * sizeof(char *));
  for (size_t i = 0; i < size; ++i) {
    auto length = strings[i].size() + 1;
    c_struct_.strings[i] = (char *)malloc(length * sizeof(char));
    strncpy(c_struct_.strings[i], strings[i].c_str(), length);
  }
}

strarray::strarray(const git_strarray *c_ptr) {
  c_struct_.count = c_ptr->count;
  c_struct_.strings = (char **)malloc(c_ptr->count * sizeof(char *));
  for (size_t i = 0; i < c_ptr->count; ++i) {
    auto length = strlen(c_ptr->strings[i]) + 1;
    c_struct_.strings[i] = (char *)malloc(length * sizeof(char));
    strncpy(c_struct_.strings[i], c_ptr->strings[i], length);
    c_struct_.strings[i][length] = '\0';
  }
}

strarray::~strarray() {
  if (c_struct_.count)
    git_strarray_free(&c_struct_);
}

strarray::strarray(strarray&& other) {
  c_struct_.count = other.c_struct_.count;
  c_struct_.strings = other.c_struct_.strings;
  other.c_struct_.count = 0;
  other.c_struct_.strings = nullptr;
}

strarray& strarray::operator= (strarray&& other) {
  if (other.c_struct_.strings != c_struct_.strings) {
    c_struct_.count = other.c_struct_.count;
    c_struct_.strings = other.c_struct_.strings;
    other.c_struct_.count = 0;
    other.c_struct_.strings = nullptr;
  }
  return *this;
}

strarray strarray::copy() const {
  return *this;
}

strarray::strarray(strarray const& other) {
  git_exception::throw_nonzero(
    git_strarray_copy(&c_struct_, &other.c_struct_));
}

std::vector<std::string> strarray::to_vector() const {
  std::vector<std::string> result{};
  for (auto tag : *this)
    result.push_back(tag);
  return result;
}

const git_strarray *strarray::c_ptr() const { return &c_struct_; }

} // namespace cppgit2
