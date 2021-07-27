package xyz.lukasz.xword.util

import it.unimi.dsi.fastutil.chars.AbstractChar2ObjectMap

fun <V, T : AbstractChar2ObjectMap<V>> T.computeIfAbsentX(key: Char, mappingFn: (Char) -> V): V {
    var value: V? = this.get(key)
    if (value == null) {
        value = mappingFn.invoke(key)
        this.put(key, value)
    }
    return value!!
}