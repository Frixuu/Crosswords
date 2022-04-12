package xyz.lukasz.xword.search

import androidx.annotation.StringRes
import xyz.lukasz.xword.R
import java.util.Locale

object WordIndexFactory {

    /**
     * Creates a new, unloaded WordIndex wrapper object.
     */
    fun create(type: WordIndexType): WordIndex {
        val locale = getLocale()
        return when (type) {
            WordIndexType.MISSING_LETTERS -> MissingLettersIndex(locale)
            else -> throw IllegalArgumentException("Unknown category name: $type")
        }
    }

    private fun getLocale(): Locale {
        return Locale("pl", "PL", "")
    }
}
