#ifndef CROSSWORD_HELPER_OBJECTS_HPP
#define CROSSWORD_HELPER_OBJECTS_HPP

#include <jni.h>

namespace interop {

    /// Creates a new Java object by calling the appropriate constructor.
    /// @param env The JNI environment.
    /// @param clazz Java class of the object to create.
    /// @param signature The signature of the constructor.
    /// @param args The arguments to the constructor.
    template <typename... Args>
    inline jobject construct(JNIEnv* env, jclass clazz, const char* sig, Args... args) {
        auto ctor = env->GetMethodID(clazz, "<init>", sig);
        return env->NewObject(clazz, ctor, args...);
    }

    /// Creates a new Java object by calling the appropriate constructor.
    /// @param env The JNI environment.
    /// @param className The name of the class.
    /// @param signature The signature of the constructor.
    /// @param args The arguments to the constructor.
    template <typename... Args>
    inline jobject construct(JNIEnv* env, const char* className, const char* sig, Args... args) {
        return construct(env, env->FindClass(className), sig, args...);
    }

    /// Creates a new Java object by calling a constructor that takes one long as its parameter.
    /// @param env The JNI environment.
    /// @param className The name of the class.
    /// @param arg0 The only argument to the constructor.
    inline jobject construct(JNIEnv* env, const char* className, jlong arg0) {
        return construct(env, className, "(J)V", arg0);
    }
}

#endif // CROSSWORD_HELPER_OBJECTS_HPP
