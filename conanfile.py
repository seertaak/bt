import os
from conans import ConanFile, CMake

class ALKConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = (
        "boost/1.72.0@_/_",
        "range-v3/0.10.0@ericniebler/stable",
        "catch2/2.11.1@_/_",
        "abseil/20200205@_/_",
        "date/2.4.1@_/_",
        "rang/3.1.0@rang/stable",
        "zmq/4.3.2@bincrafters/stable",
        "cppzmq/4.6.0@_/_",
        "xtl/0.6.12@_/_",
        "openssl/1.0.2u@_/_",
        "nlohmann_json/3.7.3@_/_",
        "xeus/0.2.38@_/_",
    )
    generators = "cmake", "virtualrunenv"
    build_policy = "missing" 

    def build(self):
        cmake = CMake(self, generator='Ninja')
        cmake.verbose = True
        cmake.configure()
        cmake.build()
        cmake.test()

