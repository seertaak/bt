#include <iostream>

#include <fstream>
#include <sstream>

#include <catch2/catch.hpp>
#include <rang.hpp>
#include <range/v3/algorithm/copy.hpp>
#include <range/v3/core.hpp>
#include <range/v3/view/tail.hpp>
#include <range/v3/view/transform.hpp>

#include <bullet/analysis/walk.hpp>
#include <bullet/lexer/lexer.hpp>
#include <bullet/lexer/token.hpp>
#include <bullet/parser/ast.hpp>
#include <bullet/parser/parser.hpp>

using namespace std;
using namespace bt;
using namespace lexer;
using namespace lexer::token;
using namespace parser;
using namespace syntax;
using namespace rang;
using namespace analysis;

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

    if (argc <= 1) return 0;

    auto source = [&] {
        auto input = ifstream(argv[1]);
        auto buffer = stringstream();
        buffer << input.rdbuf();
        return buffer.str();
    }();

    const lexer::output_t lex_output = source | tokenize;
    const syntax::tree_t ast = parser::details::parse(lex_output);

    cout << lex_output.tokens << endl;
    cout << endl << endl;

    auto s = std::stringstream();
    parser::pretty_print<empty_attribute_t>(ast, s, 0);
    cout << s.str() << endl;

    using noattr = const empty_attribute_t&;

    {
        const auto test = analysis::walk_synth<int>(
            ast, [](const string_literal_t& node, noattr) -> int { return 1; },
            [](const block_t<int>& node, noattr) -> int { return node.size(); });

        auto s = std::stringstream();
        parser::pretty_print(test, s, 0);
        cout << s.str() << endl;
    }

    return 0;
}
