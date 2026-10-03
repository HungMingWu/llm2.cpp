module;
#include <cassert>
#include <expected>
#include <filesystem>
#include <system_error>

module io;

import :impl;

using namespace io;

mapped_file::mapped_file(std::unique_ptr<mapped_file_impl> impl) noexcept : impl_(std::move(impl)) {
    assert(impl_);
}

io::mapped_file::~mapped_file() noexcept = default;

std::expected<mapped_file, std::error_code> mapped_file::open(const std::filesystem::path& path,
                                                              file_access access) {
    auto result = mapped_file_impl::open(path, access);
    if (!result)
        return std::unexpected(result.error());
    return mapped_file(std::move(*result));
}

std::expected<mapped_region, std::error_code> mapped_file::map(map_options options) const {
    assert(impl_);
    auto result = impl_->map(options);
    if (!result)
        return std::unexpected(result.error());
    return mapped_region(std::move(*result));
}

mapped_region::mapped_region(std::unique_ptr<mapped_region_impl> impl) noexcept
    : impl_(std::move(impl)) {
    assert(impl_);
}

mapped_region::~mapped_region() = default;

std::byte* mapped_region::data() noexcept {
    assert(impl_);
    return impl_->data();
}

const std::byte* mapped_region::data() const noexcept {
    assert(impl_);
    return impl_->data();
}

std::size_t mapped_region::size() const noexcept {
    assert(impl_);
    return impl_->size();
}

std::span<std::byte> mapped_region::bytes() noexcept {
    assert(impl_);
    return impl_->bytes();
}

std::span<const std::byte> mapped_region::bytes() const noexcept {
    assert(impl_);
    return impl_->bytes();
}

std::expected<std::span<std::byte>, std::error_code> mapped_region::writable_bytes() noexcept {
    assert(impl_);
    return impl_->writable_bytes();
}

std::expected<void, std::error_code> mapped_region::flush(flush_mode mode) {
    assert(impl_);
    return impl_->flush(mode);
}
