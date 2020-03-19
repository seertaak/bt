#pragma once

#include <bullet/analysis/type_checking.hpp>

namespace bt { namespace lang { namespace prelude {
    auto environment() -> bt::analysis::environment_t {
        using namespace std;
        using namespace bt;
        using namespace lexer;
        using namespace lexer::token;
        using namespace parser;
        using namespace syntax;
        using namespace rang;
        using namespace analysis;

        auto builtins = environment_t();
        builtins.context = context_t::fn;
        builtins.types.insert("i8", analysis::I8);
        builtins.types.insert("i16", analysis::I16);
        builtins.types.insert("i32", analysis::I32);
        builtins.types.insert("i64", analysis::I64);
        builtins.types.insert("u8", analysis::U8);
        builtins.types.insert("u16", analysis::U16);
        builtins.types.insert("u32", analysis::U32);
        builtins.types.insert("u64", analysis::U64);
        builtins.types.insert("f32", analysis::F32);
        builtins.types.insert("f64", analysis::F64);

        builtins.types.insert("byte", analysis::I8);
        builtins.types.insert("short", analysis::I16);
        builtins.types.insert("int", analysis::I32);
        builtins.types.insert("long", analysis::I64);

        builtins.types.insert("ubyte", analysis::U8);
        builtins.types.insert("ushort", analysis::U16);
        builtins.types.insert("uint", analysis::U32);
        builtins.types.insert("ulong", analysis::U64);

        builtins.types.insert("ptr", analysis::PTR);
        builtins.types.insert("array", analysis::ARRAY);
        builtins.types.insert("dynarr", analysis::DYNARR);
        builtins.types.insert("bool", analysis::BOOL);
        builtins.types.insert("char", analysis::CHAR);
        builtins.types.insert("slice", analysis::SLICE);
        builtins.types.insert("variant", analysis::VARIANT);
        builtins.types.insert("fn", analysis::FUNCTION);
        builtins.types.insert("tuple", analysis::TUPLE);
        builtins.types.insert("strlit", analysis::STRLIT);
        builtins.types.insert("UNKNOWN", analysis::UNKOWN);
        builtins.types.insert("void", analysis::VOID);
        builtins.types.insert("string", analysis::STRING);

        builtins.fns.insert("print", analysis::FUNCTION);

        return builtins;
    }

}}}  // namespace bt::lang::prelude
