#pragma once

#include <sstream>
#include <stdexcept>
#include <string_view>
#include <sstream>
#include <vector>
#include <memory>
#include <stdexcept>

#include <bullet/parser/location.hpp>

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
        ~raise() { error::errors.emplace_back(std::unique_ptr<std::runtime_error>(new T(msg, loc))); }
    };
    
    template <typename T, typename V>
    auto operator<<(raise<T>& r, const V& v) -> raise<T>& {
        r.msg << v;
        return r;
    }

}}  // namespace bt::analysis
