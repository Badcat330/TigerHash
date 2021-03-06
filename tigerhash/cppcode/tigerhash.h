//
// This file is a part of tigerhash project. 
// Created by Alex Glushko on 11.07.2021.
//

#ifndef TIGERHASH_TIGERHASH_H
#define TIGERHASH_TIGERHASH_H

#include <sstream>
#include <iostream>
#include "sboxes.h"

namespace tiger {
    typedef unsigned long long int word64;

    // 64 bits words for intermidiate hash values
    word64 _a;
    word64 _b;
    word64 _c;


    // Buf values
    word64 aa;
    word64 bb;
    word64 cc;

    // Global array for storing block for hash computation
    word64 _x[8];

    /**
     * Save intermidiate hash in buf values
     */ 
    void save_abc() {
        aa = _a;
        bb = _b;
        cc = _c;
    }

    /**
     * Function for computing one round of hash
     * 
     * @param a variable for swaping global _a, _b, _c values
     * @param b variable for swaping global _a, _b, _c values
     * @param c variable for swaping global _a, _b, _c values
     * @param index index of _x element for which round initiated
     * @param mul coeficient for multiplication
     */
    void round(word64& a, word64& b, word64& c, unsigned int index, char mul) {
        // convert c 64 bit value to array of bytes 
        std::string c_str = std::bitset<64>(c).to_string();
        unsigned char c_arr[8];
        for(std::size_t i = 0, j = 7; i < c_str.length(); i+=8, --j) {
            c_arr[j] = (unsigned char)std::bitset<8>(c_str.substr(i, 8)).to_ulong();
        }

        c ^= _x[index];
        a -= sboxes::t1[c_arr[0]] ^ sboxes::t2[c_arr[2]] ^ sboxes::t3[c_arr[4]] ^ sboxes::t4[c_arr[6]];
        b -= sboxes::t4[c_arr[1]] ^ sboxes::t3[c_arr[3]] ^ sboxes::t2[c_arr[5]] ^ sboxes::t1[c_arr[7]];
        b *= mul;
    }

    /**
     * Function for initiating round computation
     * 
     * @param a variable for swaping global _a, _b, _c values
     * @param b variable for swaping global _a, _b, _c values
     * @param c variable for swaping global _a, _b, _c values
     * @param mul coeficient for multiplication
     */
    void pass(word64& a, word64& b, word64& c, char mul) {
        round(a, b, c, 0, mul);
        round(b, c, a, 1, mul);
        round(c, a, b, 2, mul);
        round(a, b, c, 3, mul);
        round(b, c, a, 4, mul);
        round(c, a, b, 5, mul);
        round(a, b, c, 6, mul);
        round(b, c, a, 7, mul);
    }

    /**
     * An invertible transformation of the input data which prevent an attaker forcing sparse inputs in all three rounds 
     * All computation take place in _x array in tiger namespace
     */ 
    void key_schedule() {
        _x[0] -= _x[7] ^ 0xA5A5A5A5A5A5A5A5;
        _x[1] ^= _x[0];
        _x[2] += _x[1];
        _x[3] -= _x[2] ^ ((~_x[1]) << 19);
        _x[4] ^= _x[3];
        _x[5] += _x[4];
        _x[6] -= _x[5] ^ ((~_x[4]) >> 23);
        _x[7] ^= _x[6];
        _x[0] += _x[7];
        _x[1] -= _x[0] ^ ((~_x[7]) << 19);
        _x[2] ^= _x[1];
        _x[3] += _x[2];
        _x[4] -= _x[3] ^ ((~_x[2]) >> 23);
        _x[5] ^= _x[4];
        _x[6] += _x[5];
        _x[7] -= _x[6] ^ 0x0123456789ABCDEF;
    }


    void feedforward() {
        _a ^= aa;
        _b -= bb;
        _c += cc;
    }

    /**
     * Convert 64 bit unsigned long long int value into hex string
     * 
     * @param value of type unsigned long long int, shoul be 64 bit which will be converted in hex string
     * @return std::string in hex format with result of converting
     */ 
    std::string to_hex(word64 value) {
        std::stringstream stream;
        stream << std::hex << _b;
        std::string result(stream.str());

        while (result.length() != 16) {
            result = '0' + result;
        }

        return result;
    }

    /**
     * Compute tigerhash from string value
     * 
     * @param value std::string of any lenght, become a data for computing hash
     * @return std::string contaning computed 192 bits hash in hex format
     */ 
    std::string hash(std::string value) {
        // Basic hash initialization
        _a = 0x0123456789ABCDEF;
        _b = 0xFEDCBA9876543210;
        _c = 0xF096A5B4C3B2E187;

        // Basic buf values initialization
        aa = 0;
        bb = 0;
        cc = 0;

        // Devide message to 512 bits blocks
        while(value.length() % 64 != 0) {
            value = char(0) + value;
        }

        for(std::size_t k = 0; k < value.length(); k += 64) {

            std::string buf_str = value.substr(k, 64);

            // Devide each 512 bits block to 64 bits block and put tham in array
            for(std::size_t i = 0, j = 7; i < buf_str.length(); i+=8, --j) {
                std::string bit_str;
                std::string str_word = buf_str.substr(i, 8);
                for (int n = 0; n < 8; ++n) {
                    bit_str += std::bitset<8>(str_word[n]).to_string();
                }
                _x[j] = std::bitset<64>(bit_str).to_ullong();
            }

            // For each 512 block compute intermidiate hash values
            save_abc();
            pass(_a, _b, _c, 5);
            key_schedule();
            pass(_c, _a, _b, 7);
            key_schedule();
            pass(_b, _c, _a, 9);
            feedforward();
        }

        // Convert result in hex and return
        return to_hex(_a) + to_hex(_b) + to_hex(_c);
    }


}

#endif //TIGERHASH_TIGERHASH_H