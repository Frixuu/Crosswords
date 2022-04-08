#ifndef CROSSWORD_HELPER_POINTER_WRAPPER_HPP
#define CROSSWORD_HELPER_POINTER_WRAPPER_HPP

#include "objects.hpp"

#include <jni.h>
#include <memory>

namespace interop {

    /// Wraps existing std::shared_ptr<T> into a Java proxy object.
    /// @returns Java NativeSharedPointer object.
    /// @param env JNI environment.
    /// @param ptr shared_ptr to wrap. This method takes ownership of that pointer.
    template <typename T>
    jobject wrap_shared_ptr(JNIEnv* env, std::shared_ptr<T>&& ptr) {
        std::shared_ptr<T>* new_pointer = new std::shared_ptr<T>();
        new_pointer->swap(ptr);
        return interop::construct(env, "xyz/lukasz/xword/interop/NativeSharedPointer",
                                  reinterpret_cast<jlong>(new_pointer));
    }

    /// Unwraps Java NativeSharedPointer object into a std::shared_ptr<T>.
    /// @returns shared_ptr to unwrapped object.
    /// @param env JNI environment.
    /// @param pointer Pointer to unwrap.
    /// @details Note that this method does not validate what that proxy pointer object points to
    ///          and it is the responsibility of the caller to ensure that it is valid.
    template <typename T>
    std::shared_ptr<T> unwrap_shared_ptr(jlong pointer) {
        auto shared = *reinterpret_cast<std::shared_ptr<T>*>(pointer);
        return shared;
    }
}

#endif // CROSSWORD_HELPER_POINTER_WRAPPER_HPP
