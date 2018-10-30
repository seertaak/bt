#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>

namespace bt {
    namespace file = std::filesystem;
    using namespace std::literals;
    using clock = std::chrono::system_clock;

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
