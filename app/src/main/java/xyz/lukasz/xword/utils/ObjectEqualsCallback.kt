package xyz.lukasz.xword.utils

import androidx.recyclerview.widget.DiffUtil
import java.util.*

/**
 * An item callback that uses [Objects.equals] to compare items.
 * While this is not usually a good idea,
 * it is useful for comparing simple models, like [String]s.
 */
class ObjectEqualsCallback<T> : DiffUtil.ItemCallback<T>() {

    override fun areItemsTheSame(oldItem: T, newItem: T): Boolean {
        return Objects.equals(oldItem, newItem)
    }

    override fun areContentsTheSame(oldItem: T, newItem: T): Boolean {
        return false
    }
}
