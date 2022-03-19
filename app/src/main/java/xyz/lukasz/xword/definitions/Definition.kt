package xyz.lukasz.xword.definitions

import java.util.Locale

/**
 * Describes a word definition.
 */
data class Definition(

    /**
     * The word that is being defined.
     */
    val word: String,

    /**
     * Description what the word means.
     */
    val definition: String,

    /**
     * The language in which the definition is valid.
     */
    val locale: Locale
)
