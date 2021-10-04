#include "tone/core/errors.hpp"
#include "tone/core/tokenizer.hpp"

#include <iostream>
#include <ranges>
#include <sstream>

#include <fmt/format.h>

int main(int argc, char* argv[])
{
    using namespace tone::core;

    std::string token_prefix = "  ..";

    std::string line;
    do
    {
        fmt::print("> ");
        std::getline(std::cin, line);

        if (line.empty())
            continue;
        if (line == "exit")
            continue;

        std::istringstream ss(line);
        character_source_t input = [&ss]() { return ss.get(); };

        push_back_stream strm(input);
        try
        {
            for (auto t : tokenizer(strm))
            {
                fmt::print("    {}\n", t.dump());
            }
        }
        catch (const error& err)
        {
            ss.clear();
            ss.seekg(0);
            print_error(err, input);
        }
        fmt::print("\n");
    } while (line != "exit" && !std::cin.eof());

    return 0;
}