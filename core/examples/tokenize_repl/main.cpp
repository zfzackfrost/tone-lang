#include "tone/core/errors.hpp"
#include "tone/core/tokenize.hpp"

#include <algorithm>
#include <iostream>
#include <ranges>
#include <sstream>

int main(int argc, char* argv[])
{
    using namespace tone::core;

    std::string token_prefix = "  ..";

    std::string line;
    do
    {
        std::cout << "> ";
        std::getline(std::cin, line);

        if (line.empty())
            continue;
        if (line == "exit")
            continue;

        std::istringstream ss(line);
        character_source_t input = [&ss]() { return ss.get(); };

        try
        {
            push_back_stream strm(input);
            for (auto t = tokenize(strm); !t.is_eof(); t = tokenize(strm))
            {
                std::cout << token_prefix;
                t.dump(std::cout);
                std::cout << std::endl;
            }
        }
        catch (const error& err)
        {
            ss.clear();
            ss.seekg(0);
            format_error(err, input, std::cerr);
        }

        std::cout << std::endl;
    } while (line != "exit");

    return 0;
}