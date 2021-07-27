package xyz.lukasz.xword.dictionaries

import it.unimi.dsi.fastutil.chars.Char2ObjectAVLTreeMap
import it.unimi.dsi.fastutil.chars.Char2ObjectArrayMap
import it.unimi.dsi.fastutil.chars.Char2ObjectFunction
import it.unimi.dsi.fastutil.chars.Char2ObjectRBTreeMap

import xyz.lukasz.xword.util.computeIfAbsentX

class WordNode(
    /** If this point in the tree describes a valid word, returns that word. */
    var word: String?,
    initialMapCapacity: Int = 4
) {

    //val children = Char2ObjectAVLTreeMap<WordNode>()
    val children = Char2ObjectArrayMap<WordNode>(initialMapCapacity)

    fun pushWord(str: String, index: Int) {
        when {
            index == (str.length - 1) -> {
                children.computeIfAbsentX(str.last()) { WordNode(str, 1) }.word = str
            }
            index == str.length -> {
                word = str
            }
            index < (str.length) -> {
                children.computeIfAbsentX(str[index]) { WordNode(null, (10 - 2 * index).coerceAtLeast(3)) }.pushWord(str, index + 1)
            }
            else -> {
                throw IllegalStateException()
            }
        }
    }
}