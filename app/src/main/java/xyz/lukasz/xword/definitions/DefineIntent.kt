package xyz.lukasz.xword.definitions

import android.content.Context
import android.content.Intent
import xyz.lukasz.xword.SearchIntent
import java.text.Normalizer.Form.NFKC
import java.text.Normalizer.normalize
import java.util.*

/**
 * Intent for displaying the definition of a word.
 */
class DefineIntent(context: Context, wordToDefine: String, locale: Locale)
    : Intent(context, DefineActivity::class.java) {

    init {
        putExtra(EXTRA_WORD, normalize(wordToDefine, NFKC))
        putExtra(EXTRA_LOCALE, locale)
    }

    companion object {
        const val EXTRA_WORD = "xyz.lukasz.xword.extra.WORD_TO_DEFINE"
        const val EXTRA_LOCALE = "xyz.lukasz.xword.extra.DEFINITION_LOCALE"
    }
}
