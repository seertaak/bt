#!/bin/bash

cd tools
python3 gen_tokens.py && clang-format-11 -i tokens.hpp && cp tokens.hpp ../include/bullet/lexer/token.hpp
cd ..
