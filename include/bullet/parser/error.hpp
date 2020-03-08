#pragma once

#include <sstream>
#include <stdexcept>
#include <string_view>

#include <bullet/lexer/location.hpp>
#include <bullet/lexer/token.hpp>

namespace bt { namespace parser {
    struct error : std::runtime_error {
        error(const std::string& s, const lexer::source_token_t& t)
            : std::runtime_error([&] {
                  auto msg = std::stringstream();
                  msg << s << ", at " << t.location << ".";
                  return msg.str();
              }()) {}

        error(std::stringstream& msg, const lexer::source_token_t& t)
            : std::runtime_error([&] {
                  msg << ", at " << t.location << ".";
                  return msg.str();
              }()) {}
    };
}}  // namespace bt::parser
