package xyz.lukasz.xword.utils

import java.text.Normalizer

/**
 * Normalizes a sequence to the provided form.
 */
fun CharSequence.normalize(form: Normalizer.Form): String {
    return Normalizer.normalize(this, form)
}
