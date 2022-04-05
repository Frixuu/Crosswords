package xyz.lukasz.xword

import android.content.Intent
import java.text.Normalizer.Form.NFKC
import java.text.Normalizer.normalize

/**
 * An intent for searching for a word.
 */
class SearchIntent(pattern: String) : Intent(ACTION) {

    init {
        putExtra(EXTRA_PATTERN, normalize(pattern, NFKC))
    }

    companion object {
        const val ACTION = "xyz.lukasz.xword.action.SEARCH"
        const val EXTRA_PATTERN = "xyz.lukasz.xword.extra.PATTERN"
    }
}
