#ifndef CROSSWORD_HELPER_UTF8_HPP
#define CROSSWORD_HELPER_UTF8_HPP

#include <string>

namespace crossword::utils {

    constexpr bool codepoint_is_one_byte(const unsigned char b) {
        return b < 0b10000000;
    }

    constexpr bool codepoint_is_two_bytes(const unsigned char b) {
        return (b & 0b11000000) == 0b11000000;
    }

    constexpr bool codepoint_is_three_bytes(const unsigned char b) {
        return (b & 0b11100000) == 0b11100000;
    }

    constexpr bool codepoint_is_four_bytes(const unsigned char b) {
        return (b & 0b11110000) == 0b11110000;
    }

    constexpr bool codepoint_is_continuation(const unsigned char b) {
        return b >= 0b10000000 && b <= 0b10111111;
    }

    constexpr int codepoint_size(const unsigned char b) {
        if (codepoint_is_one_byte(b)) return 1;
        if (codepoint_is_two_bytes(b)) return 2;
        if (codepoint_is_three_bytes(b)) return 3;
        if (codepoint_is_four_bytes(b)) return 4;
        return -1;
    }

    constexpr unsigned char to_lower(const unsigned char a) {
        if (a >= 65 && a <= 90) return a + 32;
        return a;
    }
}

#endif // CROSSWORD_HELPER_UTF8_HPP
