package xyz.lukasz.xword.util

import java.util.concurrent.atomic.AtomicBoolean

inline class Spinlock(val atomicBoolean: AtomicBoolean) {

    constructor(): this(AtomicBoolean(false))

    inline fun tryLock(): Boolean {
        return atomicBoolean.compareAndSet(false, true)
    }

    inline fun lock() {
        while (!tryLock()) { }
    }

    inline fun unlock() {
        atomicBoolean.set(false)
    }

    inline fun run(block: () -> Unit) {
        lock()
        block()
        unlock()
    }
}