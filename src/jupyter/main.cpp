#include <memory>

#include <xeus/xkernel.hpp>
#include <xeus/xkernel_configuration.hpp>

#include <bullet/jupyter/interpreter.hpp>

int main(int argc, char* argv[])
{
    // Load configuration file
    std::string file_name = (argc == 1) ? "connection.json" : argv[2];
    xeus::xconfiguration config = xeus::load_configuration(file_name);

    // Create interpreter instance
    using interpreter_ptr = std::unique_ptr<bt::interpreter>;
    interpreter_ptr interpreter = interpreter_ptr(new bt::interpreter());

    // Create kernel instance and start it
    xeus::xkernel kernel(config, xeus::get_user_name(), std::move(interpreter));
    kernel.start();

    return 0;
}
