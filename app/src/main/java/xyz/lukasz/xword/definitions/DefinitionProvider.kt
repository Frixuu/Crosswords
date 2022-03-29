package xyz.lukasz.xword.definitions

/**
 * An object that provides definitions for given words.
 */
interface DefinitionProvider {
    fun getDefinitions(word: String): List<Definition>
}
