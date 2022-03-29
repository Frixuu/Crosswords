package xyz.lukasz.xword.utils

import android.view.View
import androidx.annotation.StringRes
import com.google.android.material.snackbar.Snackbar

/**
 * Creates a transient snackbar parented to this view.
 * This method is safe to call from any thread.
 * See [Snackbar] and [Snackbar.make] for more information.
 */
fun View.showSnackbar(message: CharSequence, duration: Int = Snackbar.LENGTH_LONG) {
    this.post {
        Snackbar.make(this.context, this, message, duration).show()
    }
}

/**
 * Creates a transient snackbar parented to this view.
 * This method is safe to call from any thread.
 * See [Snackbar] and [Snackbar.make] for more information.
 */
fun View.showSnackbar(@StringRes messageId: Int, duration: Int = Snackbar.LENGTH_LONG) {
    this.showSnackbar(resources.getString(messageId), duration)
}
