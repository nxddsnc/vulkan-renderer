#include <string>
#pragma once
std::string GetFileDirectory(std::string filename)
{
    std::string directory;
    const size_t last_slash_idx = filename.rfind('/');
    if (std::string::npos != last_slash_idx)
    {
        directory = filename.substr(0, last_slash_idx);
    }
    return directory;
}