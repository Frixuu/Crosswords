package xyz.lukasz.xword.dictionaries

import it.unimi.dsi.fastutil.chars.Char2ObjectArrayMap
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import xyz.lukasz.xword.util.Spinlock
import xyz.lukasz.xword.util.computeIfAbsentX

class WordNode(
    /** If this point in the tree describes a valid word, returns that word. */
    var word: String?,
) {

    private var spinlock: Spinlock? = Spinlock()
    private var children: Char2ObjectArrayMap<WordNode>? = null

    fun pushWord(str: String, index: Int) {
        if (children == null) {
            spinlock!!.run {
                children = Char2ObjectArrayMap(1)
            }
        }
        when {
            index == (str.length - 1) -> {
                children!!.computeIfAbsentX(spinlock!!, str.last()) { WordNode(str) }
                    .word = str
            }
            index < (str.length) -> {
                children!!.computeIfAbsentX(spinlock!!, str[index]) {
                    val initialCapacity = (10 - 2 * index).coerceAtLeast(1)
                    WordNode(null).apply { this.children = Char2ObjectArrayMap(initialCapacity) }
                }.pushWord(str, index + 1)
            }
            index == str.length -> {
                // Should not happen, but let's handle it anyway
                word = str
            }
            else -> {
                throw IllegalStateException()
            }
        }
    }

    suspend fun collectLocks(parallelDepth: Int = 0) {
        spinlock = null
        val deps = children?.values ?: return
        if (parallelDepth > 0) {
            coroutineScope {
                deps.forEach { launch { it.collectLocks(parallelDepth - 1) } }
            }
        } else {
            deps.forEach { it.collectLocks(0) }
        }
    }
}