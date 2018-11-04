#!/bin/bash

cd tools
python3 gen_tokens.py && clang-format-7 -i tokens.hpp && cp tokens.hpp ../include/bullet/token.hpp
