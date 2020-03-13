import os
from conans import ConanFile, CMake

class ALKConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = (
        "boost/1.71.0@conan/stable",
        "range-v3/0.10.0@ericniebler/stable",
        "Catch2/2.11.1@catchorg/stable",
        "abseil/20181200@bincrafters/stable",
        "date/2.4.1@bincrafters/stable",
        "fmt/5.2.1@bincrafters/stable",
        "rang/3.1.0@rang/stable",
    )
    generators = "cmake", "virtualrunenv"
    build_policy = "missing" 
    default_options = (
        'boost:header_only=True'
    )

    def build(self):
        cmake = CMake(self) # , generator='Ninja')
        cmake.parallel = False
        cmake.configure()
        cmake.build()
        cmake.test()

