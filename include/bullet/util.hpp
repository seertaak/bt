#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <utility>
#include <variant>

#include <boost/hana.hpp>

namespace bt {
    namespace file = std::filesystem;
    using namespace std::literals;
    using clock = std::chrono::system_clock;

    BOOST_HOF_STATIC_LAMBDA_FUNCTION(match) = boost::hof::pipable([](auto&& x, auto&&... fn) {
        return std::visit(boost::hana::overload(std::forward(fn)...), std::forward(x));
    });

    auto match2 = [](auto&& x, auto&&... fn) {
        return std::visit(boost::hana::overload(std::forward(fn)...), std::forward(x));
    };

    class ephemeral_file {
        const file::path file_;

    public:
        ephemeral_file(const std::string& contents)
            : file_(file::temp_directory_path() /
                    ("f"s + std::to_string(clock::now().time_since_epoch().count()) + ".tmp")) {
            if (!empty(contents)) {
                try {
                    auto out = std::fstream(file_, std::ios::out);
                    out << contents;
                } catch (...) {
                    if (exists(file_)) {
                        remove(file_);
                    }
                }
            }
        }

        ~ephemeral_file() {
            if (exists(file_)) {
                remove(file_);
            }
        }

        auto get() const -> file::path { return file_; }
        operator file::path() const { return file_; }
    };
}  // namespace bt
