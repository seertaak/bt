#pragma once

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <vector>

#include <bullet/parser/location.hpp>
#include <boost/stacktrace.hpp>
#include <boost/exception/all.hpp>

typedef boost::error_info<struct tag_stacktrace, boost::stacktrace::stacktrace> traced;


namespace bt { namespace analysis {
    struct error : std::runtime_error {
        error(const std::string& s, const parser::location_t& l);
        error(std::stringstream& msg, const parser::location_t& l);

        static std::vector<std::unique_ptr<std::runtime_error>> errors;
    };

    template <typename T>
    struct raise {
        std::stringstream msg;
        parser::location_t loc;

        template <typename Node>
        raise(const Node& node) : loc(node.get().location) {}
        ~raise() {
            msg << "\n\n" << boost::stacktrace::stacktrace() << std::endl << std::endl;

            error::errors.emplace_back(std::unique_ptr<std::runtime_error>(new T(msg, loc)));
        }
    };

    template <typename T, typename V>
    auto operator<<(raise<T>& r, const V& v) -> raise<T>& {
        r.msg << v;
        return r;
    }

}}  // namespace bt::analysis
