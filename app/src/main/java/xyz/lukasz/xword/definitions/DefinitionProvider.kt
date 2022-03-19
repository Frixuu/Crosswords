package xyz.lukasz.xword.definitions

import java.util.*

interface DefinitionProvider {
    fun getDefinitions(word: String): List<Definition>
}
