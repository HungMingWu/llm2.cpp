module;
#define NOMINMAX
#include <windows.h>

#include <cerrno>
#include <expected>
#include <filesystem>
#include <limits>
#include <memory>
#include <system_error>
#include <utility>

module io:impl;
import io;

namespace io {
namespace {
std::error_code windows_error() noexcept {
    return {static_cast<int>(::GetLastError()), std::system_category()};
}

std::error_code invalid_argument_error() noexcept {
    return std::make_error_code(std::errc::invalid_argument);
}
} // namespace

class mapped_region::mapped_region_impl {
  public:
    mapped_region_impl() noexcept = default;

    mapped_region_impl(void* base, std::byte* data, std::size_t mapping_size, std::size_t size,
                       HANDLE file, bool copy_on_write, bool writable) noexcept
        : mapping_base_(base), data_(data), mapping_size_(mapping_size), size_(size), file_(file),
          copy_on_write_(copy_on_write), writable_(writable) {}

    mapped_region_impl(mapped_region_impl&& other) noexcept
        : mapping_base_(std::exchange(other.mapping_base_, nullptr)),
          data_(std::exchange(other.data_, nullptr)),
          mapping_size_(std::exchange(other.mapping_size_, 0)),
          size_(std::exchange(other.size_, 0)),
          file_(std::exchange(other.file_, INVALID_HANDLE_VALUE)),
          copy_on_write_(std::exchange(other.copy_on_write_, false)),
          writable_(std::exchange(other.writable_, false)) {}

    mapped_region_impl& operator=(mapped_region_impl&& other) noexcept {
        if (this != &other) {
            release();
            mapping_base_ = std::exchange(other.mapping_base_, nullptr);
            data_ = std::exchange(other.data_, nullptr);
            mapping_size_ = std::exchange(other.mapping_size_, 0);
            size_ = std::exchange(other.size_, 0);
            file_ = std::exchange(other.file_, INVALID_HANDLE_VALUE);
            copy_on_write_ = std::exchange(other.copy_on_write_, false);
            writable_ = std::exchange(other.writable_, false);
        }
        return *this;
    }

    mapped_region_impl(mapped_region_impl const&) = delete;
    mapped_region_impl& operator=(mapped_region_impl const&) = delete;

    ~mapped_region_impl() {
        release();
    }

    template <typename Self> [[nodiscard]] auto data(this Self&& self) noexcept {
        return self.data_;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

    template <typename Self> [[nodiscard]] std::span<std::byte> bytes(this Self&& self) noexcept {
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
        if (!writable_ || !mapping_base_)
            return {};
        if (!::FlushViewOfFile(mapping_base_, mapping_size_))
            return std::unexpected(windows_error());
        if (mode == flush_mode::sync && !::FlushFileBuffers(file_))
            return std::unexpected(windows_error());
        return {};
    }

  private:
    void release() noexcept {
        if (mapping_base_)
            ::UnmapViewOfFile(mapping_base_);
        if (file_ != INVALID_HANDLE_VALUE)
            ::CloseHandle(file_);
        mapping_base_ = nullptr;
        data_ = nullptr;
        mapping_size_ = 0;
        size_ = 0;
        file_ = INVALID_HANDLE_VALUE;
        copy_on_write_ = false;
        writable_ = false;
    }

    void* mapping_base_ = nullptr;
    std::byte* data_ = nullptr;
    std::size_t mapping_size_ = 0;
    std::size_t size_ = 0;
    HANDLE file_ = INVALID_HANDLE_VALUE;
    bool copy_on_write_ = false;
    bool writable_ = false;
};

class mapped_file::mapped_file_impl {
  public:
    mapped_file_impl(HANDLE file, bool writable) noexcept : file_(file), writable_(writable) {}

    mapped_file_impl(mapped_file_impl&& other) noexcept
        : file_(std::exchange(other.file_, INVALID_HANDLE_VALUE)),
          writable_(std::exchange(other.writable_, false)) {}

    mapped_file_impl& operator=(mapped_file_impl&& other) noexcept {
        if (this != &other) {
            if (file_ != INVALID_HANDLE_VALUE)
                ::CloseHandle(file_);
            file_ = std::exchange(other.file_, INVALID_HANDLE_VALUE);
            writable_ = std::exchange(other.writable_, false);
        }
        return *this;
    }

    mapped_file_impl(mapped_file_impl const&) = delete;
    mapped_file_impl& operator=(mapped_file_impl const&) = delete;

    ~mapped_file_impl() noexcept {
        if (file_ != INVALID_HANDLE_VALUE)
            ::CloseHandle(file_);
    }

    [[nodiscard]] static std::expected<std::unique_ptr<mapped_file_impl>, std::error_code>
    open(std::filesystem::path const& path, file_access access = file_access::read) {
        if (access != file_access::read && access != file_access::read_write)
            return std::unexpected(invalid_argument_error());

        const bool writable = access == file_access::read_write;
        const DWORD desired_access = writable ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
        HANDLE file = ::CreateFileW(path.c_str(), desired_access,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE)
            return std::unexpected(windows_error());
        return std::make_unique<mapped_file_impl>(file, writable);
    }

    std::expected<std::unique_ptr<mapped_region::mapped_region_impl>, std::error_code>
    map(map_options options) const {
        if (file_ == INVALID_HANDLE_VALUE)
            return std::unexpected(std::make_error_code(std::errc::bad_file_descriptor));
        if (options.access != map_access::read && options.access != map_access::read_write &&
            options.access != map_access::copy_on_write)
            return std::unexpected(invalid_argument_error());
        if (options.access == map_access::read_write && !writable_)
            return std::unexpected(std::make_error_code(std::errc::permission_denied));

        LARGE_INTEGER file_size{};
        if (!::GetFileSizeEx(file_, &file_size))
            return std::unexpected(windows_error());
        if (file_size.QuadPart < 0 ||
            options.offset > static_cast<std::uint64_t>(file_size.QuadPart))
            return std::unexpected(invalid_argument_error());

        const auto remaining = static_cast<std::uint64_t>(file_size.QuadPart) - options.offset;
        const auto requested = options.size.value_or(remaining);
        if (requested == 0 || requested > remaining ||
            requested > std::numeric_limits<std::size_t>::max())
            return std::unexpected(invalid_argument_error());

        SYSTEM_INFO system_info{};
        ::GetSystemInfo(&system_info);
        const auto granularity = static_cast<std::uint64_t>(system_info.dwAllocationGranularity);
        if (granularity == 0)
            return std::unexpected(invalid_argument_error());
        const auto aligned_offset = options.offset - options.offset % granularity;
        const auto delta = static_cast<std::size_t>(options.offset - aligned_offset);
        if (requested > std::numeric_limits<std::size_t>::max() - delta)
            return std::unexpected(std::make_error_code(std::errc::value_too_large));
        const auto mapping_size = static_cast<std::size_t>(requested) + delta;

        DWORD protection = PAGE_READONLY;
        DWORD desired_view_access = FILE_MAP_READ;
        if (options.access == map_access::read_write) {
            protection = PAGE_READWRITE;
            desired_view_access = FILE_MAP_READ | FILE_MAP_WRITE;
        } else if (options.access == map_access::copy_on_write) {
            protection = PAGE_WRITECOPY;
            desired_view_access = FILE_MAP_COPY;
        }

        HANDLE mapping = ::CreateFileMappingW(file_, nullptr, protection, 0, 0, nullptr);
        if (!mapping)
            return std::unexpected(windows_error());

        void* base =
            ::MapViewOfFile(mapping, desired_view_access, static_cast<DWORD>(aligned_offset >> 32),
                            static_cast<DWORD>(aligned_offset & 0xffffffffu), mapping_size);
        const DWORD map_error = base ? ERROR_SUCCESS : ::GetLastError();
        ::CloseHandle(mapping);
        if (!base)
            return std::unexpected(
                std::error_code(static_cast<int>(map_error), std::system_category()));

        HANDLE region_file = INVALID_HANDLE_VALUE;
        if (!::DuplicateHandle(::GetCurrentProcess(), file_, ::GetCurrentProcess(), &region_file, 0,
                               FALSE, DUPLICATE_SAME_ACCESS)) {
            const auto error = windows_error();
            ::UnmapViewOfFile(base);
            return std::unexpected(error);
        }

        auto* data = static_cast<std::byte*>(base) + delta;
        return std::make_unique<mapped_region::mapped_region_impl>(
            base, data, mapping_size, static_cast<std::size_t>(requested), region_file,
            options.access == map_access::copy_on_write, options.access != map_access::read);
    }

  private:
    HANDLE file_ = INVALID_HANDLE_VALUE;
    bool writable_ = false;
};
} // namespace io
