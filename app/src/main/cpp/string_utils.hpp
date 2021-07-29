#ifndef CROSSWORD_HELPER_STRING_UTILS_HPP
#define CROSSWORD_HELPER_STRING_UTILS_HPP

#include <string>
#include <jni.h>

namespace crossword::utils {

    /// Copies contents of a Java string into a new std::string.
    std::string string_from_java(JNIEnv *env, jstring jstr) {
        if (jstr != nullptr) {
            auto char_ptr = env->GetStringUTFChars(jstr, 0);
            auto str = std::string(char_ptr);
            env->ReleaseStringUTFChars(jstr, char_ptr);
            return str;
        } else {
            return std::string();
        }
    }

    constexpr bool codepoint_is_one_byte(const char b) {
        return (b & 0b10000000) != 0b10000000;
    }

    constexpr bool codepoint_is_two_bytes(const char b) {
        return (b & 0b11000000) == 0b11000000;
    }

    constexpr bool codepoint_is_three_bytes(const char b) {
        return (b & 0b11100000) == 0b11100000;
    }

    constexpr bool codepoint_is_four_bytes(const char b) {
        return (b & 0b11110000) == 0b11110000;
    }

    constexpr int codepoint_size(const char b) {
        if (codepoint_is_one_byte(b)) return 1;
        if (codepoint_is_two_bytes(b)) return 2;
        if (codepoint_is_three_bytes(b)) return 3;
        if (codepoint_is_four_bytes(b)) return 4;
        return -1;
    }
}

#endif //CROSSWORD_HELPER_STRING_UTILS_HPP
