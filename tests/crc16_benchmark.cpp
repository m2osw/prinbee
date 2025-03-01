
#include    <cstdint>
#include    <iostream>
#include    <vector>
#include    <snapdev/timespec_ex.h>
#include    <sys/times.h>

// g++ -O3 -I../../BUILD/Debug/dist/include -std=gnu++23 -o a tests/crc16_benchmark.cpp


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


constexpr std::uint16_t const g_crc16_tbl[] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040,
};


// $crc is an integer between 0 and 0xFFFF
// $dataByte is an integer between 0 and 0xFF
// The result is an integer between 0 and 0xFFFF
std::uint16_t add_crc(std::uint16_t crc, std::uint8_t byte)
{
    int const index((crc & 0xFF) ^ byte);
    std::uint16_t const crc16int(g_crc16_tbl[index]);
    return (crc >> 8) ^ crc16int;
}

// $buffer is a string containing the binary data
// The result is an integer between 0 and 0xFFFF
std::uint16_t crc16_calc(std::vector<std::uint8_t> const & buffer)
{
    std::uint16_t crc16(0);
    std::size_t max(buffer.size());
    std::uint8_t const * p(buffer.data());
    for(; max > 0; --max, ++p)
    {
        crc16 = add_crc(crc16, *p);
    }
    return crc16;
}

int main(int argc, char * argv[])
{
    std::size_t const max(atoi(argv[1]));
    int const repeat(atoi(argv[2]));

    // create a buffer
    //
    srand(time(nullptr));
    std::vector<std::uint8_t> buffer(max);
    for(std::size_t i(0); i < max; ++i)
    {
        buffer[i] = rand();
    }

    tms a_crc16_start;
    times(&a_crc16_start);
std::cout << "a start when read " << a_crc16_start.tms_utime << "\n";
    //snapdev::timespec_ex a_crc16_start(snapdev::now());
    for(std::size_t i(0); i < repeat; ++i)
    {
        gen_crc16(buffer.data(), max);
    }
    //snapdev::timespec_ex a_crc16_end(snapdev::now());
    tms a_crc16_end;
    times(&a_crc16_end);
std::cout << "a start when read end: " << a_crc16_start.tms_utime << "\n";
std::cout << "a end when read " << a_crc16_end.tms_utime << "\n";

    tms b_crc16_start;
    times(&b_crc16_start);
    //snapdev::timespec_ex b_crc16_start(snapdev::now());
    for(std::size_t i(0); i < repeat; ++i)
    {
        std::uint16_t const crc16(crc16_calc(buffer));

        std::vector<std::uint8_t> copy(buffer);
        copy.push_back(crc16);
        copy.push_back(crc16 >> 8);
        std::uint16_t const zero(crc16_calc(copy));
        if(zero != 0)
        {
            std::cerr << "error: not zero!?\n";
            return 1;
        }
    }
    //snapdev::timespec_ex b_crc16_end(snapdev::now());
    tms b_crc16_end;
    times(&b_crc16_end);

    clock_t const a(a_crc16_end.tms_utime - a_crc16_start.tms_utime);
    clock_t const b(b_crc16_end.tms_utime - b_crc16_start.tms_utime);

    std::cout
        << "a start " << a_crc16_start.tms_utime
        << " -> a end " << a_crc16_end.tms_utime
        << " (ticks: " << sysconf(_SC_CLK_TCK)
        << ")\n";
    std::cout
        << "b start " << b_crc16_start.tms_utime
        << " -> b end " << b_crc16_end.tms_utime
        << "\n";
    std::cout
        << "a = "
        << a
        << ", b = "
        << b
        << "\n";

    return 0;
}



// vim: ts=4 sw=4 et
