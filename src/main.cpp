#include <iostream>

#include <rang.hpp>

using namespace std;
using namespace rang;

constexpr auto title = R"(
     / /_  __  __/ / /__  / /_
    / __ \/ / / / / / _ \/ __/
   / /_/ / /_/ / / /  __/ /_    
  /_.___/\__,_/_/_/\___/\__/
)"sv;

int main(int argc, const char* argv[]) {
    cout << style::bold << fg::blue << title << style::reset << style::bold << fg::green
         << style::italic << "\n  fast. " << fg::blue << "expressive." << style::blink << fg::red
         << " dangerous.\n"
         << style::reset << fg::reset << endl;
    return 0;
}
