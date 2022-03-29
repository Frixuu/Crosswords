package xyz.lukasz.xword.native

import org.jetbrains.annotations.Contract

/**
 * Represents a pointer to a std::shared_ptr (pointer to a pointer) in native code.
 */
class NativeSharedPointer(ptr: Long) : NativePointer(ptr) {

    @Contract(pure = false)
    override fun freePointer(ptr: Long) {
        freeImpl(ptr)
    }

    companion object {

        @JvmStatic external fun freeImpl(ptr: Long)

        @JvmStatic
        @Contract(" -> new", pure = true)
        fun nil(): NativeSharedPointer {
            return NativeSharedPointer(0L)
        }
    }
}
