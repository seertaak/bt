#include <bullet/analysis/error.hpp>

namespace bt { namespace analysis {
    using namespace std;

    error::error(const string& s, const parser::location_t& l)
            : runtime_error([&] {
                  auto msg = stringstream();
                  msg << s << ", at " << l << ".";
                  return msg.str();
              }()) {}

    error::error(stringstream& msg, const parser::location_t& l)
            : runtime_error([&] {
                  msg << ", at " << l << ".";
                  return msg.str();
              }()) {}

    vector<unique_ptr<runtime_error>> error::errors;
}}
