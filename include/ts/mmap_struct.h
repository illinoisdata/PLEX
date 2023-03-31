#ifndef MMAP_STRUCT_
#define MMAP_STRUCT_

#include <algorithm>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <iterator>
#include <cstddef>

#ifndef __has_include
  static_assert(false, "__has_include not supported");
#else
#  if __cplusplus >= 201703L && __has_include(<filesystem>)
#    include <filesystem>
     namespace fs = std::filesystem;
#  elif __has_include(<experimental/filesystem>)
#    include <experimental/filesystem>
     namespace fs = std::experimental::filesystem;
#  elif __has_include(<boost/filesystem.hpp>)
#    include <boost/filesystem.hpp>
     namespace fs = boost::filesystem;
#  endif
#endif

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/unique_ptr.hpp>

namespace mmap_struct {

/* LazyVector: mmap-based array */

template<class K>
class LazyVector {
 public:
  class Iterator;

  LazyVector() : size_(0), begin_(NULL), fd_(-1), addr_(NULL), file_size_(0) {}
  LazyVector(const LazyVector& other) = delete;
  LazyVector(LazyVector&& other) = delete;
  LazyVector& operator=(const LazyVector& other) = delete;
  LazyVector& operator=(LazyVector&& other) {
    // Move info over
    this->size_ = other.size_;
    this->begin_ = other.begin_;
    this->fd_ = other.fd_;
    this->addr_ = other.addr_;
    this->file_size_ = other.file_size_;

    // Reset other info
    other.size_ = 0;
    other.begin_ = NULL;
    other.fd_ = -1;
    other.addr_ = NULL;
    other.file_size_ = 0;

    return *this;
  }

  LazyVector(const std::vector<K>& source, fs::path filepath) {  // Build new file from vector
    const char* filename = filepath.c_str();
    size_t data_size = source.size();
    size_t file_size = data_size * sizeof(K);

    // prepare directory
    if (!fs::is_directory(filepath.parent_path()) || !fs::exists(filepath.parent_path())) {
      fs::create_directories(filepath.parent_path());
      std::cout << "Created directory " << filepath.parent_path() << std::endl;
    }

    // open file
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
      int errnum = errno;
      std::cerr << "Error write-opening " << filename << ": " << strerror(errnum) << std::endl;
      exit(1);
    }

    // allocate space on the file
    int fa = fallocate(fd, 0 /*allocating disk space*/, 0, file_size);
    if (fa != 0) {
      int errnum = errno;
      std::cerr << "Error allocating space: " << strerror(errnum) << std::endl;
      exit(1);
    }

    // mmap
    void* addr = mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
      int errnum = errno;
      std::cerr << "Error mmap data of size " << file_size << " bytes" << ": " << strerror(errnum) << std::endl;
      exit(1);
    }
    K* whole_data = reinterpret_cast<K*>(addr);

    // write element to mmap-ed array
    for (size_t idx = 0; idx < data_size; ++idx) {
      whole_data[idx] = source[idx];
    }
    msync(addr, file_size, MS_SYNC);
    std::cout << "Written to " << filepath << " with size " << file_size << " bytes, at " << addr << std::endl;

    // assign to class
    this->size_ = data_size;
    this->begin_ = whole_data;
    this->fd_ = fd;
    this->addr_ = addr;
    this->file_size_ = file_size;
  }

  LazyVector(fs::path filepath, const size_t data_size) {  // Load from existing file
    const char* filename = filepath.c_str();

    // open file
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
      std::cerr << "Error read-opening " << filename << std::endl;
      exit(1);
    }

    // get file size for mapping
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
      std::cerr << "Error obtaining fstat" << std::endl;
      exit(1);
    }
    size_t file_size = sb.st_size;

    // mmap
    void* addr = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
      std::cerr << "Error mmap data" << std::endl;
      exit(1);
    }
    K* whole_data = reinterpret_cast<K*>(addr);
    std::cout << "Mmap-ed " << filepath << " with size " << file_size << " bytes, at " << addr << std::endl;

    // assign to class
    this->size_ = data_size;
    this->begin_ = whole_data;
    this->fd_ = fd;
    this->addr_ = addr;
    this->file_size_ = file_size;
  }

  ~LazyVector() {
    if (this->addr_ != NULL) {
      munmap(this->addr_, this->file_size_);
      // std::cout << "Closed mmap at " << addr_ << std::endl;
    }
    if (this->fd_ != -1) {
      close(this->fd_);
      // std::cout << "Closed fd " << fd_ << std::endl;
    }
  }

  Iterator begin() const {
    return Iterator(this->begin_, this->begin_, this->begin_ + this->size_);
  }

  Iterator end() const {
    return Iterator(this->begin_ + this->size_, this->begin_, this->begin_ + this->size_);
  }

  K& operator[](int index) const {
    return this->begin_[index];
  }

  size_t size() const {
    return this->size_;
  }

  size_t into_file(const char* filename) {
    throw std::runtime_error("LazyVector does not implement into_file.");
  }

  K& front() const {
    return (*this)[0];
  }

  K& back() const {
    return (*this)[this->size() - 1];
  }

 private:

  size_t size_;
  K* begin_;

  // for closing
  int fd_;
  void* addr_;
  size_t file_size_;
};

template<class K>
class LazyVector<K>::Iterator {
  public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type   = std::ptrdiff_t;
  using value_type        = K;
  using pointer           = K*;
  using reference         = K&;

  Iterator() noexcept : ptr_(nullptr), begin_(nullptr), end_(nullptr) {}
  Iterator(K* ptr, K* begin, K* end) noexcept : ptr_(ptr), begin_(begin), end_(end) {}

  K& operator*() noexcept {
    return *(this->ptr_);
  }

  K* operator->() {
    return this->ptr_;
  }

  Iterator& operator++() noexcept {  // ++it
    if (this->ptr_ != nullptr) {
      ptr_++;
    }
    return *this;
  }

  Iterator operator++(int) noexcept {  // it++
    Iterator temp = *this;
    ++(*this);
    return temp;
  }

  Iterator operator+(int jump) noexcept {
    // std::cout << "Jumpping " << jump << ", " << this->ptr_ + jump << " [" << this->begin_ << ", " << this->end_ << "]" << std::endl;
    return Iterator(this->ptr_ + jump, this->begin_, this->end_);
  }

  bool operator==(const Iterator &other) {
    return this->ptr_ == other.ptr_;
  }

  bool operator!=(const Iterator &other) {
    return this->ptr_ != other.ptr_;
  }

  private:
  K* ptr_;
  K* begin_;
  K* end_;
};

}  // mmap_struct

#endif  // MMAP_STRUCT_H_