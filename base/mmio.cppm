module;
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <system_error>

export module io;

export namespace io {
enum class file_access { read, read_write };
enum class map_access { read, read_write, copy_on_write };
enum class flush_mode { async, sync };
struct map_options {
    std::uint64_t offset = 0;
    std::optional<std::uint64_t> size = std::nullopt;
    map_access access = map_access::read;
};
class mapped_file;

class mapped_region {
  public:
    // Opaque implementation type used by the platform module partition.
    class mapped_region_impl;

  private:
    friend class mapped_file;
    std::unique_ptr<mapped_region_impl> impl_;
    mapped_region(std::unique_ptr<mapped_region_impl>) noexcept;

  public:
    mapped_region(mapped_region&&) noexcept = default;
    mapped_region& operator=(mapped_region&&) noexcept = default;

    mapped_region(mapped_region const&) = delete;
    mapped_region& operator=(mapped_region const&) = delete;

    ~mapped_region();

    [[nodiscard]]
    std::byte* data() noexcept;

    [[nodiscard]]
    const std::byte* data() const noexcept;

    [[nodiscard]]
    std::size_t size() const noexcept;

    [[nodiscard]]
    std::span<std::byte> bytes() noexcept;

    [[nodiscard]]
    std::span<const std::byte> bytes() const noexcept;

    std::expected<std::span<std::byte>, std::error_code> writable_bytes() noexcept;

    std::expected<void, std::error_code> flush(flush_mode = flush_mode::sync);
};

class mapped_file {
  private:
    class mapped_file_impl;
    std::unique_ptr<mapped_file_impl> impl_;
    mapped_file(std::unique_ptr<mapped_file_impl>) noexcept;

  public:
    mapped_file(mapped_file&&) noexcept = default;
    mapped_file& operator=(mapped_file&&) noexcept = default;

    mapped_file(const mapped_file&) = delete;
    mapped_file& operator=(const mapped_file&) = delete;

    ~mapped_file() noexcept;
    [[nodiscard]]
    static std::expected<mapped_file, std::error_code> open(const std::filesystem::path& path,
                                                            file_access access = file_access::read);

    std::expected<mapped_region, std::error_code> map(map_options = {}) const;
};
} // namespace io
