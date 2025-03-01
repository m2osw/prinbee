
// test crc16 from library

#include    <fstream>
#include    <iostream>
#include    <vector>

#include    "../prinbee/network/crc16.cpp"

int main()
{
    std::ifstream in("a.bin");

    if(!in.is_open())
    {
        std::cerr << "error: could not open a.bin\n";
        return 1;
    }

    std::vector<std::uint8_t> data(100);
    in.read(reinterpret_cast<char *>(data.data()), 100);

    if(!in)
    {
        std::cerr << "error: reading file failed.\n";
        return 1;
    }

    std::uint16_t const result(prinbee::gen_crc16(data.data(), 100));
    std::cout << "crc16 = " << result << "\n";

    return 0;
}

// vim: ts=4 sw=4 et
