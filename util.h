#pragma once
#include "framework.h"

namespace util
{
    static std::vector<uint8_t> read_rom(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            std::cout << "Failed to open file: " << path << std::endl;
            return {};
        }

        std::vector<uint8_t> buffer;
        buffer.resize(file.seekg(0, std::ios::end).tellg());
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        file.close();
        return buffer;
    }

    constexpr unsigned int str2int(const char *str, int h = 0)
    {
        return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
    }
}