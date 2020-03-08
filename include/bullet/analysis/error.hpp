#pragma once

#include <sstream>
#include <stdexcept>
#include <string_view>

#include <bullet/parser/location.hpp>

namespace bt { namespace analysis {
    struct error : std::runtime_error {
        error(const std::string& s, const parser::location_t& l)
            : std::runtime_error([&] {
                  auto msg = std::stringstream();
                  msg << s << ", at " << l << ".";
                  return msg.str();
              }()) {}

        error(std::stringstream& msg, const parser::location_t& l)
            : std::runtime_error([&] {
                  msg << ", at " << l << ".";
                  return msg.str();
              }()) {}
    };
}}  // namespace bt::analysis
