#include <iostream>
#include <string>

#include <iostream>

#include <array>
#include <fstream>
#include <sstream>

#include <catch2/catch.hpp>
#include <rang.hpp>
#include <range/v3/core.hpp>
#include <range/v3/view/zip.hpp>

#include <bullet/analysis/error.hpp>
#include <bullet/analysis/prelude_environment.hpp>
#include <bullet/analysis/symtab.hpp>
#include <bullet/analysis/type.hpp>
#include <bullet/analysis/type_checking.hpp>
#include <bullet/analysis/walk.hpp>
#include <bullet/banner.hpp>
#include <bullet/jupyter/interpreter.hpp>
#include <bullet/lexer/lexer.hpp>
#include <bullet/lexer/token.hpp>
#include <bullet/parser/ast.hpp>
#include <bullet/parser/parser.hpp>

namespace nl = nlohmann;
using namespace std;

namespace views = ranges::views;

using namespace std;
using namespace bt;
using namespace lexer;
using namespace lexer::token;
using namespace parser;
using namespace syntax;
using namespace rang;
using namespace analysis;

template <typename T>
auto print_kind(raise<T>& err, auto invoc, auto type) -> raise<T>& {
    if (type && invoc)
        err << "template";
    else if (type && !invoc)
        err << "type";
    else if (!type && invoc)
        err << "function";
    else
        err << "variable";
    return err;
};

namespace bt {

nl::json interpreter::execute_request_impl(int execution_counter,      // Typically the cell number
                                           const std::string& source,  // Code to execute
                                           bool /*silent*/,
                                           bool /*store_history*/,
                                           nl::json /*user_expressions*/,
                                           bool /*allow_stdin*/) {
    // You can use the C-API of your target language for executing the code,
    // e.g. `PyRun_String` for the Python C-API
    //      `luaL_dostring` for the Lua C-API

    // Use this method for publishing the execution result to the client,
    // this method takes the ``execution_counter`` as first argument,
    // the data to publish (mime type data) as second argument and metadata
    // as third argument.
    // Replace "Hello World !!" by what you want to be displayed under the execution cell

    analysis::error::errors.clear();

    try {
        const lexer::output_t lex_output = source | tokenize;
        const syntax::tree_t ast = parser::details::parse(lex_output);

        attr_node_t<analysis::type_t> typed_ast =
            walk_post_order<analysis::type_t>(ast, [](auto, auto) { return analysis::type_t(); });

        auto builtins = lang::prelude::environment();
        type_check(typed_ast, builtins);

        if (analysis::error::errors.empty()) {
            auto s = stringstream();

            s << fg::cyan << "Tokens:" << style::reset << endl;
            s << lex_output.tokens << endl;
            s << endl << endl;

            s << fg::cyan << "AST:" << style::reset << endl;
            parser::pretty_print<empty_attribute_t>(ast, s, 0);

            s << fg::cyan << "Typed AST:" << style::reset << endl;
            parser::pretty_print(typed_ast.get(), s, 0);

            nl::json pub_data;
            pub_data["text/plain"] = s.str();

            publish_execution_result(execution_counter, std::move(pub_data), nl::json::object());
        } else {
            auto s = stringstream();
            s << fg::red << "ERRORS:";
            for (auto&& e : analysis::error::errors) s << e->what() << endl;
            s << style::reset << endl;

            publish_execution_error("Error.", "Program error", {s.str()});
        }
    } catch (const runtime_error& e) {
        auto s = stringstream();
        s << fg::red << "Fatal error: " << e.what() << endl;
        s << fg::red << "Normal errors error:";
        for (auto&& e : analysis::error::errors) s << e->what() << endl;
        s << style::reset << endl;

        publish_execution_error("Error.", "Compiler Runtime Excpetion", {s.str()});
    } catch (...) {
        auto s = stringstream();
        s << fg::red << "UNKNOWN FATAL ERROR." << endl;
        s << fg::red << "Normal errors:";
        for (auto&& e : analysis::error::errors) s << e->what() << endl;
        s << style::reset << endl;

        publish_execution_error("Error.", "Compiler Exception", {s.str()});
    }

    // You can also use this method for publishing errors to the client, if the code
    // failed to execute

    nl::json result;
    result["status"] = "ok";
    return result;
}

void interpreter::configure_impl() {
    // Perform some operations
}

nl::json interpreter::complete_request_impl(const std::string& code, int cursor_pos) {
    nl::json result;

    // Code starts with 'H', it could be the following completion
    if (code[0] == 'H') {
        result["status"] = "ok";
        result["matches"] = {"Hello", "Hey", "Howdy"};
        result["cursor_start"] = 5;
        result["cursor_end"] = cursor_pos;
    }
    // No completion result
    else {
        result["status"] = "ok";
        result["matches"] = nl::json::array();
        result["cursor_start"] = cursor_pos;
        result["cursor_end"] = cursor_pos;
    }

    return result;
}

nl::json interpreter::inspect_request_impl(const std::string& code,
                                           int /*cursor_pos*/,
                                           int /*detail_level*/) {
    nl::json result;

    if (code.compare("print") == 0) {
        result["found"] = true;
        result["text/plain"] = "Print objects to the text stream file, [...]";
    } else {
        result["found"] = false;
    }

    result["status"] = "ok";
    return result;
}

nl::json interpreter::is_complete_request_impl(const std::string& /*code*/) {
    nl::json result;

    // if (is_complete(code))
    // {
    result["status"] = "complete";
    // }
    // else
    // {
    //    result["status"] = "incomplete";
    //    result["indent"] = 4;
    //}

    return result;
}

nl::json interpreter::kernel_info_request_impl() {
    nl::json result;
    result["implementation"] = "bullet";
    result["implementation_version"] = "0.1.0";
    result["language_info"]["name"] = "bullet";
    result["language_info"]["version"] = "0.1";
    result["language_info"]["mimetype"] = "text/x-python";
    result["language_info"]["file_extension"] = ".bt";
    result["implementation_version"] = "0.1.0";
    std::string banner = bt::banner.data();
    result["banner"] = banner;
    return result;
}

void interpreter::shutdown_request_impl() {
    std::cout << "Bye!!" << std::endl;
}

}  // namespace bt
