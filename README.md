# Bullet

## The principles of Bullet.

Bullet is a compiled, generic programming language that is optimized for 
hobby usage, solo developers, or small teams, in a performance-critical
setting where full (i.e. manual) control over memory and threads is
required. Its overarching philosophy
is to trust the programmer, rather than imposing restrictions on them.
For example, Bullet adopts the open extension approaches of Julia and Go:
an extension written by a user of a library is syntactically on an equal
footing to the functionality of the library itself.
Instead of trying to divine the possible uses of a language feature, 
Bullet strives to give the programmer the tools to "skin" those language
features in a way they see fit. This is facilitated via an underlying
syntax which is inspired by s-expressions. However, Bullet is also inspired
by whitespace-sensitive notations such as Python and Markdown, which
over the last decade have proven popular among both programmers and 
scientists alike. Bullet draws inpiration from libraries such
as LLVM and higher level wrappers of those like CPython, Numba, and other
languages such as Julia. Bullet sees AOT compilation as just simply a JIT
run at compile time whose binary output is stored for later execution; 
Bullet supports two modes of execution: interpreted and machine code.

We care about:

- fast
- open, extensible
- power
- intuitive
- simple
- orthogonal design

We believe that:

- good design is emergent
- the programmer can be trusted
- the language designer is just another user

We don't care about:

- safety
- stupid programmers
- OO
- dependent types, and other exotic type features

## Interpreted Mode

- offers the dynamism of Python
    - a record types's very fields are mutable; it becomes simply
      a dictionary
    - you can manipulate syntax, programmatically create new 
      definitions, etc.
    - But unlike Python, this language is still
        1. non-latently typed.
        2. uses value semantics
- any code headed by a "meta" tag is run in the interpreter
- if that code block is itself within a block that is compiled, then 
    - an instance of the interpreter is bundled in the compiled code.
    - an automatic bridge is created so that variables in the compiled
      code are visible within the interpreted code
