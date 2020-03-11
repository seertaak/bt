#pragma once

#include <chrono>
#include <fstream>
#include <utility>
#include <variant>

#include <boost/filesystem.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/hana.hpp>
#include <boost/hof.hpp>
#include <boost/type_index.hpp>

namespace bt {
namespace file = boost::filesystem;
using namespace std::literals;
using clock = std::chrono::system_clock;

auto match = [](auto&& x, auto&&... fn) {
    return std::visit(boost::hana::overload(std::forward(fn)...), std::forward(x));
};

template <typename Fn>
using lambda_arg_t = std::remove_const_t<std::remove_reference_t<
    typename boost::mpl::at_c<boost::function_types::parameter_types<decltype(&Fn::operator())>,
                              1>::type>>;

class ephemeral_file {
    const file::path file_;

public:
    ephemeral_file(const std::string& contents)
        : file_(file::temp_directory_path() /
                ("f"s + std::to_string(clock::now().time_since_epoch().count()) + ".tmp")) {
        if (!empty(contents)) {
            try {
                auto out = file::fstream(file_, std::ios::out);
                out << contents;
            } catch (...) {
                if (exists(file_)) {
                    remove(file_);
                }
            }
        }
    }

    ~ephemeral_file() {
        if (exists(file_)) {
            remove(file_);
        }
    }

    auto get() const -> file::path { return file_; }
    operator file::path() const { return file_; }
};

template <typename T>
using ptr = std::shared_ptr<T>;

template <typename T>
struct ref {
    ptr<T> value;

    ref(const T& t) : value{std::make_shared<T>(t)} {}

    ref() : value{std::make_shared<T>()} {}
    ref(const ref&) = default;
    ref& operator=(const ref&) = default;
    ref& operator=(const T& t) {
        *value = t;
        return *this;
    }

    T& get() { return *value; }
    const T& get() const { return *value; }

    operator T&() { return *value; }
    operator const T&() const { return *value; }

    template <typename U>
    inline auto is() const -> bool {
        return std::holds_alternative<U>(*value);
    }

    template <typename U>
    inline auto is() -> bool {
        return std::holds_alternative<U>(*value);
    }

    template <typename U>
    inline auto as() const -> const U& {
        return std::get<U>(*value);
    }

    template <typename U>
    inline auto as() -> U& {
        return std::get<U>(*value);
    }

    template <typename U>
    inline auto get_if() const -> const U* {
        return std::get_if<U>(&*value);
    }

    template <typename U>
    inline auto get_if() -> U* {
        return std::get_if<U>(&*value);
    }
};

template <typename T>
auto operator==(const ref<T>& l, const ref<T>& r) -> bool {
    return l.get() == r.get();
}

template <typename T>
auto operator==(const ref<T>& l, const T& r) -> bool {
    return l.get() == r;
}

template <typename T>
auto operator==(const T& l, const ref<T>& r) -> bool {
    return l == r.get();
}

template <typename T>
auto operator!=(const ref<T>& l, const ref<T>& r) -> bool {
    return !(l == r);
}

template <typename T>
auto operator!=(const ref<T>& l, const T& r) -> bool {
    return !(l == r);
}

template <typename T>
auto operator!=(const T& l, const ref<T>& r) -> bool {
    return !(l == r);
}

template <typename Node, typename Attr>
struct attr {
    using node_type = Node;
    using attribute_type = Attr;

    const Node& node;
    Attr& attribute;
};
}  // namespace bt
