
// test crc16 from library

#include    <fstream>
#include    <iostream>
#include    <vector>

#include    "../prinbee/network/crc16.cpp"

// this version (which generate a different number from the crc16_compute()
// function...) is about half the speed compare to the other CRC16 function
// so we keep the other one; also when in Debug mode (i.e. not optimized
// with at least -O2) it is actually more like 100 times slower.
//
std::uint16_t gen_crc16(std::uint8_t const * data, std::size_t size)
{
    if(data == nullptr)
    {
        return 0;
    }

    constexpr std::uint16_t CRC16 = 0x8005;

    std::uint16_t crc = 0;
    std::int_fast8_t bits_read = 0;
    while(size > 0)
    {
        int const bit_flag(crc >> 15);

        // get next bit
        //
        crc <<= 1;
        crc |= (*data >> bits_read) & 1; // work from the least significant bits

        // increment bit counter
        //
        ++bits_read;
        if(bits_read > 7)
        {
            bits_read = 0;
            ++data;
            --size;
        }

        // cycle check
        //
        if(bit_flag != 0)
        {
            crc ^= CRC16;
        }
    }

    // push the last 16 bits
    //
    for(int i(0); i < 16; ++i)
    {
        int const bit_flag(crc >> 15);
        crc <<= 1;
        if(bit_flag != 0)
        {
            crc ^= CRC16;
        }
    }

    return crc;
}



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

    // this function was first in the crc16.cpp file in prinbee; but
    // I removed it to avoid later confusion
    //
    std::uint16_t const result(/*prinbee::*/ gen_crc16(data.data(), 100));
    std::cout << "crc16 = " << result << "\n";

    return 0;
}

// vim: ts=4 sw=4 et
