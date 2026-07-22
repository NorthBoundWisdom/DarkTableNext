#include <iostream>
#include <span>
#include <string_view>
#include <vector>

#include "ravo/cli/application.h"

int main(const int argc, char *argv[])
{
    auto engine = ravo::EngineFacade::create_phase1();
    if (!engine)
    {
        std::cerr << "ravo: " << ravo::error_code_name(engine.error().code) << ": "
                  << engine.error().message << '\n';
        return ravo::cli_exit_code(engine.error().code);
    }

    std::vector<std::string_view> arguments;
    arguments.reserve(static_cast<std::size_t>(argc > 0 ? argc - 1 : 0));
    for (int index = 1; index < argc; ++index)
    {
        arguments.emplace_back(argv[index]);
    }
    const ravo::CliApplication application(engine.value(), std::cout, std::cerr);
    return application.run(std::span{arguments});
}
