package xyz.lukasz.xword

import android.app.Activity
import android.content.res.AssetManager
import xyz.lukasz.xword.interop.NativeSharedPointer
import java.text.Collator
import java.util.*

/**
 * A Dictionary is a wrapper for indices that allow for quick word lookup.
 */
class Dictionary(private val lang: String, private val country: String) {

    /**
     * Represents information about this dictionary's language.
     */
    val locale: Locale = Locale(lang, country)

    /**
     * Collator for this dictionary's locale.
     * Used for string comparisons.
     */
    val collator: Collator = Collator.getInstance(locale)

    /**
     * A pointer to a native Dictionary object.
     */
    private var pointer = NativeSharedPointer.nil()

    /**
     * Returns true if the internal pointer
     * points to a valid native Dictionary object.
     */
    val ready: Boolean get() = !pointer.nil

    /**
     * Attempts to load an internal asset
     * associated with this Dictionary's locale.
     */
    fun loadFromAsset(activity: Activity) {
        loadFromAsset(activity.assets)
    }

    /**
     * Attempts to load an internal asset
     * associated with this Dictionary's locale.
     */
    fun loadFromAsset(assetManager: AssetManager) {
        loadFromAsset(assetManager, "dictionaries/${lang}_${country}/words.txt")
    }

    /**
     * Attempts to load an internal asset under a specified path.
     */
    fun loadFromAsset(assetManager: AssetManager, filename: String) {
        pointer.setPointer(0L)
        val threadCount = Runtime.getRuntime().availableProcessors()
        pointer = loadNative(assetManager, filename, threadCount)
        if (pointer.nil) {
            throw Exception("Native loading failed")
        }
    }

    /**
     * Finds all matching a provided pattern.
     * For example, a pattern ".ró." would match "drób", "próg" and "król".
     * @param limit How many strings can be returned at most, a negative value means all of them.
     */
    fun findPartial(pattern: String, cursor: String? = null, limit: Int = -1): Array<String> {
        return if (ready) {
            findPartialNative(pointer.getPointer(), pattern.lowercase(), cursor, limit)
        } else {
            emptyArray()
        }
    }

    /**
     * A native method that attempts to load and index a native Dictionary
     * and returns a pointer to that object
     * or null, if the operation failed.
     */
    private external fun loadNative(
        assetManager: AssetManager,
        filename: String,
        concLevel: Int
    ): NativeSharedPointer

    /**
     * A native method that traverses a native Dictionary indices
     * and returns an array of words that match a specified pattern.
     */
    private external fun findPartialNative(
        nativePtr: Long,
        pattern: String,
        cursor: String?,
        limit: Int
    ): Array<String>

    companion object {
        var current: Dictionary? = null
    }
}
