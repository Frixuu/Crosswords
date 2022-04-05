package xyz.lukasz.xword.utils

import androidx.recyclerview.widget.DefaultItemAnimator
import androidx.recyclerview.widget.RecyclerView.ItemAnimator

/**
 * Contains utilities for creating and manipulating [ItemAnimator]s.
 */
object Animators {

    /**
     * Creates a [DefaultItemAnimator] with all the animation durations set to one duration.
     * @param duration The duration of the animation, in milliseconds.
     */
    fun defaultWithDuration(duration: Long): ItemAnimator {
        return DefaultItemAnimator().apply {
            addDuration = duration
            removeDuration = duration
            moveDuration = duration
            changeDuration = duration
        }
    }
}
