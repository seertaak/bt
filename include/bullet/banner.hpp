#pragma once

#include <iostream>
#include <sstream>

#include <rang.hpp>

namespace bt {

using namespace std::literals;

constexpr auto banner = R"(
     / /_  __  __/ / /__  / /_
    / __ \/ / / / / / _ \/ __/
   / /_/ / /_/ / / /  __/ /_    
  /_.___/\__,_/_/_/\___/\__/
)"sv;

inline auto coloured_banner() -> std::string {
    using namespace rang;

    auto s = std::stringstream();
    s << style::bold << fg::blue << banner << style::reset << style::bold << fg::green
      << style::italic << "\n  fast. " << fg::blue << "expressive." << style::blink << fg::red
      << " dangerous.\n"
      << style::reset << fg::reset;
    return s.str();
}
}  // namespace bt
