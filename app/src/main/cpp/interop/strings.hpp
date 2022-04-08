#ifndef CROSSWORD_HELPER_STRINGS_HPP
#define CROSSWORD_HELPER_STRINGS_HPP

#include <jni.h>
#include <string>

namespace interop {

    /// Copies contents of a Java string into a new std::string.
    std::u8string copy_utf8_string(JNIEnv* env, jstring jstr) {
        if (jstr != nullptr) {
            auto char_ptr = env->GetStringUTFChars(jstr, 0);
            auto str = std::u8string(reinterpret_cast<const char8_t*>(char_ptr));
            env->ReleaseStringUTFChars(jstr, char_ptr);
            return str;
        } else {
            return u8"";
        }
    }
}

#endif // CROSSWORD_HELPER_STRINGS_HPP
