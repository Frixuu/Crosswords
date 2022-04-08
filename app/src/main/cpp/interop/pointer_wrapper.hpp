#ifndef CROSSWORD_HELPER_POINTER_WRAPPER_HPP
#define CROSSWORD_HELPER_POINTER_WRAPPER_HPP

#include <jni.h>
#include <memory>

namespace interop {

    /// Wraps existing std::shared_ptr<T> into a JNI proxy object.
    template<typename T>
    jobject wrap_shared_ptr(JNIEnv* env, std::shared_ptr<T>&& ptr) {
        std::shared_ptr<T>* new_ptr = new std::shared_ptr<T>();
        new_ptr->swap(ptr);
        auto clazz = env->FindClass("xyz/lukasz/xword/interop/NativeSharedPointer");
        auto ctor = env->GetMethodID(clazz, "<init>", "(J)V");
        return env->NewObject(clazz, ctor, reinterpret_cast<jlong>(new_ptr));
    }

    template<typename T>
    std::shared_ptr<T> unwrap_shared_ptr(jlong pointer) {
        auto shared = *reinterpret_cast<std::shared_ptr<T>*>(pointer);
        return shared;
    }
}

#endif // CROSSWORD_HELPER_POINTER_WRAPPER_HPP
