module;
#include <algorithm>
#include <charconv>
#include <concepts>
#include <cstddef>
#include <meta>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

export module cli;

export namespace cli {
template <std::size_t N> struct fixed_string {
    char value[N];
    consteval fixed_string(const char (&text)[N]) {
        std::copy_n(text, N, value);
    }
};

template <std::size_t N> struct long_name {
    fixed_string<N> value;

    consteval long_name(const char (&text)[N]) : value(text) {}
};

struct short_name {
    char value;
};

template <class T> struct parsed_value {
    T value;
    std::size_t consumed;
};

template <typename T> constexpr bool is_optional_v = false;

template <typename T> constexpr bool is_optional_v<std::optional<T>> = true;

template <class T> parsed_value<T> parse_value(std::span<char*> argv) {
    if (argv.empty())
        throw std::runtime_error("missing option value");

    std::string_view text = argv.front();
    T value{};
    if constexpr (is_optional_v<T>) {
        using Value = typename T::value_type;
        auto parsed = parse_value<Value>(argv);
        return {T{std::move(parsed.value)}, parsed.consumed};
    } else if constexpr (std::same_as<T, std::string>) {
        value = text;
    } else if constexpr (std::same_as<T, std::string_view>) {
        value = text;
    } else if constexpr (std::integral<T>) {
        auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
        if (ec != std::errc{} || ptr != text.data() + text.size())
            throw std::runtime_error("invalid option value: " + std::string(text));
    } else if constexpr (std::floating_point<T>) {
        auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
        if (ec != std::errc{} || ptr != text.data() + text.size())
            throw std::runtime_error("invalid option value: " + std::string(text));
    } else if constexpr (std::is_class_v<T>) {
        std::size_t consumed = 0;
        template for (constexpr auto member :
                      std::define_static_array(std::meta::nonstatic_data_members_of(
                          ^^T, std::meta::access_context::unchecked()))) {
            using Member = std::remove_cvref_t<decltype(value.[:member:])>;
            auto parsed = parse_value<Member>(argv.subspan(consumed));
            value.[:member:] = std::move(parsed.value);
            consumed += parsed.consumed;
        }
        return {std::move(value), consumed};
    } else {
        static_assert(false, "Unsupported CLI argument type");
    }
    return {std::move(value), 1};
}

template <class Tag> consteval std::string_view get_annotation_text(const Tag& tag) {
    return std::string_view(std::define_static_string(tag.value.value));
}

template <std::meta::info Member> consteval std::string_view get_long_name() {
    template for (constexpr auto ann :
                  std::define_static_array(std::meta::annotations_of(Member))) {
        constexpr auto annotation_type = std::meta::type_of(std::meta::constant_of(ann));

        constexpr auto unqualified_type = std::meta::remove_cv(annotation_type);
        if constexpr (std::meta::has_template_arguments(unqualified_type)) {
            if constexpr (std::meta::template_of(unqualified_type) == ^^cli::long_name) {
                return get_annotation_text(std::meta::extract<typename[:annotation_type:]>(ann));
            }
        }
    }

    return std::meta::identifier_of(Member);
}

template <std::meta::info Member> consteval char get_short_name() {
    for (auto ann : std::meta::annotations_of_with_type(Member, ^^short_name)) {
        return std::meta::extract<short_name>(ann).value;
    }

    auto name = std::meta::identifier_of(Member);
    return name.empty() ? '\0' : name.front();
}

template <class Args> Args parse(std::span<char*> argv) {
    Args result{};

    while (!argv.empty()) {
        std::string_view arg = argv.front();
        argv = argv.subspan(1);

        bool matched = false;

        template for (constexpr auto member :
                      std::define_static_array(std::meta::nonstatic_data_members_of(
                          ^^Args, std::meta::access_context::unchecked()))) {
            constexpr auto lname = get_long_name<member>();

            constexpr char sname = get_short_name<member>();

            auto& field = result.[:member:];

            using T = std::remove_cvref_t<decltype(field)>;

            //
            // --foo
            //
            if (arg.starts_with("--")) {
                auto name = arg.substr(2);

                if (name == lname) {
                    matched = true;

                    if constexpr (std::same_as<T, bool>) {
                        field = true;
                    } else if constexpr (is_optional_v<T>) {
                        using Value = typename T::value_type;
                        auto parsed = parse_value<Value>(argv);
                        argv = argv.subspan(parsed.consumed);
                        field = std::move(parsed.value);
                    } else {
                        auto parsed = parse_value<T>(argv);
                        argv = argv.subspan(parsed.consumed);
                        field = std::move(parsed.value);
                    }
                }
            }

            //
            // -f
            //
            else if (arg.size() == 2 && arg[0] == '-' && arg[1] == sname) {
                matched = true;

                if constexpr (std::same_as<T, bool>) {
                    field = true;
                } else if constexpr (is_optional_v<T>) {
                    using Value = typename T::value_type;
                    auto parsed = parse_value<Value>(argv);
                    argv = argv.subspan(parsed.consumed);
                    field = std::move(parsed.value);
                } else {
                    auto parsed = parse_value<T>(argv);
                    argv = argv.subspan(parsed.consumed);
                    field = std::move(parsed.value);
                }
            }
        }

        if (!matched) {
            throw std::runtime_error("unknown option: " + std::string(arg));
        }
    }

    return result;
}

} // namespace cli
