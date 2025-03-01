
// g++ -o a tests/crc16_generate_random_file.cpp
//
#include    <fstream>
#include    <iostream>

int main()
{
    std::ofstream out("a.bin");

    if(!out.is_open())
    {
        std::cerr << "error: could not open a.bin\n";
        return 1;
    }

    srand(time(nullptr));
    for(int i(0); i < 100; ++i)
    {
        char c(rand());
        out.write(&c, 1);
    }

    if(!out)
    {
        std::cerr << "error: an error occurred while generating the file.\n";
        return 1;
    }

    return 0;
}

// vim: ts=4 sw=4 et
