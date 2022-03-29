package xyz.lukasz.xword.native

import java.util.concurrent.atomic.AtomicLong

/**
 * A wrapper for a pointer to a native object.
 */
abstract class NativePointer(ptr: Long = 0L) : AutoCloseable {

    /**
     * The pointer to the native object.
     */
    private val pointer = AtomicLong(ptr)

    fun getPointer(): Long {
        return pointer.get()
    }

    /**
     * Is this pointer a nullptr?
     */
    val nil: Boolean
        get() = getPointer() == 0L

    /**
     * Sets the new pointer, freeing the previous one if necessary.
     */
    fun setPointer(new: Long) {
        val old = pointer.getAndSet(new)
        if (old != 0L) {
            freePointer(old)
        }
    }

    /**
     * Frees the native object.
     */
    open fun free() {
        setPointer(0L)
    }

    /**
     * Frees the object at the given pointer.
     */
    protected abstract fun freePointer(ptr: Long)

    fun finalize() {
        free()
    }

    override fun close() {
        free()
    }
}
