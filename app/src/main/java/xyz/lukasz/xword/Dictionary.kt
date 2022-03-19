package xyz.lukasz.xword

import android.app.Activity
import android.content.res.AssetManager
import java.text.Collator
import java.util.*

/**
 * A Dictionary is a wrapper for indices that allow for quick word lookup.
 */
class Dictionary(private val lang: String, private val country: String) {

    /**
     * Represents information about this dictionary's language.
     */
    private val locale: Locale = Locale(lang, country)

    /**
     * Collator for this dictionary's locale.
     * Used for string comparisons.
     */
    val collator: Collator = Collator.getInstance(locale)

    /**
     * A pointer to a native Dictionary object.
     */
    private var nativePtr: Long = 0

    /**
     * Returns true if the internal pointer
     * points to a valid native Dictionary object.
     */
    val loaded: Boolean get() = nativePtr != 0L

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
        val oldPtr = nativePtr
        nativePtr = 0
        val threadCount = Runtime.getRuntime().availableProcessors()
        val ptr = loadNative(assetManager, filename, oldPtr, threadCount)
        if (ptr == 0L) {
            throw Exception("Native loading failed")
        }
        nativePtr = ptr
    }

    /**
     * Finds all matching a provided pattern.
     * For example, a pattern ".ró." would match "drób", "próg" and "król".
     * @param limit How many strings can be returned at most, a negative value means all of them.
     */
    fun findPartial(pattern: String, cursor: String? = null, limit: Int = -1): Array<String> {
        return if (loaded) {
            findPartialNative(nativePtr, pattern.lowercase(), cursor, limit)
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
        oldPtr: Long,
        concLevel: Int
    ): Long

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
        init {
            System.loadLibrary("native-lib")
        }
    }
}
