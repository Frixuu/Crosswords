package xyz.lukasz.xword

import android.text.TextWatcher

/**
 * A special case of [TextWatcher] that does not care where the change has been made.
 */
fun interface TextChangedListener : TextWatcher {
    override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {}
    override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {}
}
