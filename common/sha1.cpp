#include "sha1.h"
#include <cstring> 
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

namespace chat 
{

    
    const uint32_t sha1::K[4] = 
    {
        0x5A827999, 
        0x6ED9EBA1,
        0x8F1BBCDC,
        0xCA62C1D6 
    };

    uint32_t sha1::leftRotate(uint32_t value, unsigned int shift) 
    {
        return (value << shift) | (value >> (32 - shift));
    }

    void sha1::processBlock(const MessageBlock& block, HashState& state) 
    {
        uint32_t w[80];

        for (int i = 0; i < 16; ++i) 
        {
            w[i] = (static_cast<uint32_t>(block[i * 4]) << 24) |
                (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
                (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
                (static_cast<uint32_t>(block[i * 4 + 3]));
        }

        for (int t = 16; t < 80; ++t) 
        {
            w[t] = leftRotate(w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16], 1);
        }

        uint32_t a = state[0];
        uint32_t b = state[1];
        uint32_t c = state[2];
        uint32_t d = state[3];
        uint32_t e = state[4];

        for (int t = 0; t < 80; ++t) 
        {
            uint32_t f, k;
            if (t < 20) 
            {
                f = (b & c) | ((~b) & d);
                k = K[0];
            }
            else if (t < 40) 
            {
                f = b ^ c ^ d;
                k = K[1];
            }
            else if (t < 60) 
            {
                f = (b & c) | (b & d) | (c & d);
                k = K[2];
            }
            else 
            {
                f = b ^ c ^ d;
                k = K[3];
            }

            uint32_t temp = leftRotate(a, 5) + f + e + k + w[t];
            e = d;
            d = c;
            c = leftRotate(b, 30);
            b = a;
            a = temp;
        }

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
    }

    std::string sha1::hash(const std::string& input) 
    {
        std::vector<uint8_t> message(input.begin(), input.end());
        size_t original_length = message.size();

        message.push_back(0x80);

        while ((message.size() * 8) % 512 != 448) 
        {
            message.push_back(0x00);
        }

        uint64_t original_bit_length = static_cast<uint64_t>(original_length) * 8;
        for (int i = 7; i >= 0; --i) 
        {
            message.push_back(static_cast<uint8_t>((original_bit_length >> (i * 8)) & 0xFF));
        }

        HashState state = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };

        for (size_t i = 0; i < message.size(); i += BLOCK_SIZE) 
        {
            MessageBlock block;
            std::memcpy(block, message.data() + i, BLOCK_SIZE);
            processBlock(block, state);
        }

        uint8_t final_hash[HASH_SIZE];
        for (int i = 0; i < 5; ++i) 
        {
            final_hash[i * 4] = static_cast<uint8_t>((state[i] >> 24) & 0xFF);
            final_hash[i * 4 + 1] = static_cast<uint8_t>((state[i] >> 16) & 0xFF);
            final_hash[i * 4 + 2] = static_cast<uint8_t>((state[i] >> 8) & 0xFF);
            final_hash[i * 4 + 3] = static_cast<uint8_t>((state[i]) & 0xFF);
        }

        return toHex(final_hash, HASH_SIZE);
    }

    std::string sha1::toHex(const uint8_t* data, size_t len) 
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (size_t i = 0; i < len; ++i) 
        {
            ss << std::setw(2) << static_cast<unsigned int>(data[i]);
        }
        return ss.str();
    }

} 