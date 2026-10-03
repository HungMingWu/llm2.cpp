module;
#include <cerrno>
#include <expected>
#include <fcntl.h>
#include <filesystem>
#include <limits>
#include <memory>
#include <sys/mman.h>
#include <sys/stat.h>
#include <system_error>
#include <unistd.h>
#include <utility>

module io:impl;
import io;

namespace io {
class mapped_region::mapped_region_impl {
  public:
    mapped_region_impl() noexcept = default;

    mapped_region_impl(mapped_region_impl&& other) noexcept
        : mapping_base_(std::exchange(other.mapping_base_, nullptr)),
          data_(std::exchange(other.data_, nullptr)),
          mapping_size_(std::exchange(other.mapping_size_, 0)),
          size_(std::exchange(other.size_, 0)),
          copy_on_write_(std::exchange(other.copy_on_write_, false)),
          writable_(std::exchange(other.writable_, false)) {}
    mapped_region_impl& operator=(mapped_region_impl&& other) noexcept {
        if (this != &other) {
            if (mapping_base_ != nullptr)
                ::munmap(mapping_base_, mapping_size_);
            mapping_base_ = std::exchange(other.mapping_base_, nullptr);
            data_ = std::exchange(other.data_, nullptr);
            mapping_size_ = std::exchange(other.mapping_size_, 0);
            size_ = std::exchange(other.size_, 0);
            copy_on_write_ = std::exchange(other.copy_on_write_, false);
            writable_ = std::exchange(other.writable_, false);
        }
        return *this;
    }

    mapped_region_impl(mapped_region_impl const&) = delete;
    mapped_region_impl& operator=(mapped_region_impl const&) = delete;

    ~mapped_region_impl() {
        if (mapping_base_ != nullptr)
            ::munmap(mapping_base_, mapping_size_);
    }

    mapped_region_impl(void* base, std::byte* data, std::size_t mapping_size, std::size_t size,
                       bool copy_on_write, bool writable) noexcept
        : mapping_base_(base), data_(data), mapping_size_(mapping_size), size_(size),
          copy_on_write_(copy_on_write), writable_(writable) {}

    template <typename Self>
    [[nodiscard]]
    auto data(this Self&& self) noexcept {
        return self.data_;
    }

    [[nodiscard]]
    std::size_t size() const noexcept {
        return size_;
    }

    template <typename Self>
    [[nodiscard]]
    std::span<std::byte> bytes(this Self&& self) noexcept {
        return {self.data_, self.size_};
    }

    std::expected<std::span<std::byte>, std::error_code> writable_bytes() noexcept {
        if (!writable_)
            return std::unexpected(std::make_error_code(std::errc::permission_denied));
        return std::span<std::byte>{data_, size_};
    }

    std::expected<void, std::error_code> flush(flush_mode mode = flush_mode::sync) {
        if (copy_on_write_)
            return std::unexpected(std::make_error_code(std::errc::operation_not_supported));
        const int flags = mode == flush_mode::async ? MS_ASYNC : MS_SYNC;
        if (mapping_base_ && ::msync(mapping_base_, mapping_size_, flags) == -1)
            return std::unexpected(std::error_code(errno, std::system_category()));
        return {};
    }

  private:
    void* mapping_base_ = nullptr;
    std::byte* data_ = nullptr;
    std::size_t mapping_size_ = 0;
    std::size_t size_ = 0;
    bool copy_on_write_ = false;
    bool writable_ = false;
};

class mapped_file::mapped_file_impl {
  public:
    mapped_file_impl(int handle, bool writable) noexcept : handle_(handle), writable_(writable) {}
    mapped_file_impl(mapped_file_impl&& other) noexcept
        : handle_(std::exchange(other.handle_, -1)),
          writable_(std::exchange(other.writable_, false)) {}
    mapped_file_impl& operator=(mapped_file_impl&& other) noexcept {
        if (this != &other) {
            if (handle_ != -1)
                ::close(handle_);
            handle_ = std::exchange(other.handle_, -1);
            writable_ = std::exchange(other.writable_, false);
        }
        return *this;
    }

    mapped_file_impl(const mapped_file_impl&) = delete;
    mapped_file_impl& operator=(const mapped_file_impl&) = delete;

    ~mapped_file_impl() noexcept {
        if (handle_ != -1)
            ::close(handle_);
    }

    [[nodiscard]]
    static std::expected<std::unique_ptr<mapped_file_impl>, std::error_code>
    open(const std::filesystem::path& path, file_access access = file_access::read) {
        if (access != file_access::read && access != file_access::read_write)
            return std::unexpected(std::make_error_code(std::errc::invalid_argument));
        const bool writable = access == file_access::read_write;
        const int flags = writable ? O_RDWR : O_RDONLY;
        const int handle = ::open(path.c_str(), flags);
        if (handle == -1)
            return std::unexpected(std::error_code(errno, std::system_category()));
        return std::make_unique<mapped_file_impl>(handle, writable);
    }

    std::expected<std::unique_ptr<io::mapped_region::mapped_region_impl>, std::error_code>
    map(map_options options) const {
        if (options.access != map_access::read && options.access != map_access::read_write &&
            options.access != map_access::copy_on_write)
            return std::unexpected(std::make_error_code(std::errc::invalid_argument));
        if (options.access == map_access::read_write && !writable_)
            return std::unexpected(std::make_error_code(std::errc::permission_denied));
        struct stat info {};
        if (::fstat(handle_, &info) == -1)
            return std::unexpected(std::error_code(errno, std::system_category()));
        if (info.st_size < 0 || options.offset > static_cast<std::uint64_t>(info.st_size))
            return std::unexpected(std::error_code(EINVAL, std::system_category()));

        const auto remaining = static_cast<std::uint64_t>(info.st_size) - options.offset;
        const auto requested = options.size.value_or(remaining);
        if (requested > remaining || requested > std::numeric_limits<std::size_t>::max())
            return std::unexpected(std::error_code(EINVAL, std::system_category()));
        if (requested == 0)
            return std::unexpected(std::error_code(EINVAL, std::system_category()));

        errno = 0;
        const long page_size = ::sysconf(_SC_PAGE_SIZE);
        if (page_size <= 0)
            return std::unexpected(std::error_code(errno ? errno : EINVAL, std::system_category()));
        const auto page = static_cast<std::uint64_t>(page_size);
        const auto aligned_offset = options.offset - options.offset % page;
        if (aligned_offset > static_cast<std::uint64_t>(std::numeric_limits<off_t>::max()))
            return std::unexpected(std::error_code(EOVERFLOW, std::system_category()));
        const auto delta = static_cast<std::size_t>(options.offset - aligned_offset);
        if (requested > std::numeric_limits<std::size_t>::max() - delta)
            return std::unexpected(std::error_code(EOVERFLOW, std::system_category()));
        const auto mapping_size = static_cast<std::size_t>(requested) + delta;

        int protection = PROT_READ;
        int flags = MAP_SHARED;
        switch (options.access) {
        case map_access::read:
            break;
        case map_access::read_write:
            protection |= PROT_WRITE;
            break;
        case map_access::copy_on_write:
            protection |= PROT_WRITE;
            flags = MAP_PRIVATE;
            break;
        }
        void* base = ::mmap(nullptr, mapping_size, protection, flags, handle_,
                            static_cast<off_t>(aligned_offset));
        if (base == MAP_FAILED)
            return std::unexpected(std::error_code(errno, std::system_category()));
        auto* data = static_cast<std::byte*>(base) + delta;
        return std::make_unique<io::mapped_region::mapped_region_impl>(
            base, data, mapping_size, static_cast<std::size_t>(requested),
            options.access == map_access::copy_on_write, options.access != map_access::read);
    }

  private:
    int handle_;
    bool writable_;
};
} // namespace io
