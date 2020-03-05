#pragma once

#include <sstream>
#include <stdexcept>
#include <string_view>

#include <bullet/lexer/location.hpp>
#include <bullet/lexer/token.hpp>

namespace bt { namespace lexer {
    struct error : std::runtime_error {
        error(const std::string& s, uint32_t line, uint16_t col)
            : std::runtime_error([&] {
                  auto msg = std::stringstream();
                  msg << " at " << line << ":" << col;
                  return msg.str();
              }()) {}

        error(std::stringstream& msg, uint32_t line, uint16_t col)
            : std::runtime_error([&] {
                  msg << " at " << line << ":" << col;
                  return msg.str();
              }()) {}
    };
}}  // namespace bt::lexer
