#ifndef CROSSWORD_HELPER_ANDROID_HPP
#define CROSSWORD_HELPER_ANDROID_HPP

#include <android/log.h>
#include <jni.h>
#include <string>

namespace crossword::utils::android {

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

    template<typename ...Args>
    [[maybe_unused]] inline void logf(android_LogPriority priority,
                                      const char *tag,
                                      const char *fmt,
                                      Args... args) {

        __android_log_print(priority, tag, fmt, args...);
    }

    template<typename ...Args>
    [[maybe_unused]] inline void infof(const char *tag,
                                       const char *fmt,
                                       Args... args) {

        logf(ANDROID_LOG_INFO, tag, fmt, args...);
    }

    template<typename ...Args>
    [[maybe_unused]] inline void infof(const char *fmt,
                                       Args... args) {

        infof("Native", fmt, args...);
    }
}

#endif //CROSSWORD_HELPER_ANDROID_HPP
