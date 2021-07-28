package xyz.lukasz.xword.util

import it.unimi.dsi.fastutil.chars.AbstractChar2ObjectMap
import java.util.concurrent.locks.ReentrantReadWriteLock
import kotlin.concurrent.read
import kotlin.concurrent.write

inline fun <V, T : AbstractChar2ObjectMap<V>> T.computeIfAbsentX(
    spinlock: Spinlock,
    key: Char,
    mappingFn: (Char) -> V
): V {

    var value: V?
    value = this.get(key)
    if (value == null) {
        value = mappingFn.invoke(key)
        spinlock.run {
            if (this.get(key) == null) {
                this.put(key, value)
            }
        }
    }
    return value!!
}