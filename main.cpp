#include <iostream>
#include <filesystem>

int main() {
    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator{{"include/mcmapper"}}) {
        auto path = dir_entry.path();

        auto begin = path.end();
        while (*--begin != "mcmapper");
        
        std::filesystem::path relpath;
        while (++begin != path.end()) relpath /= *begin;

        std::cout << relpath.replace_extension() << std::endl;
    }

    return 0;
}