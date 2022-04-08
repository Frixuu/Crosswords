package xyz.lukasz.xword.search

import android.content.Context
import android.content.res.AssetManager
import xyz.lukasz.xword.interop.NativeSharedPointer
import java.text.Normalizer
import java.util.*

/**
 * MissingLettersIndex is an index that provides lookup of words,
 * where the matched pattern can have some of its letters missing.
 */
class MissingLettersIndex(lang: String, country: String)
    : WordIndex(Locale(lang, country)) {

    /**
     * Attempts to load an internal asset
     * associated with this Dictionary's locale.
     */
    fun loadFromAsset(context: Context) {
        val path = "dictionaries/${locale.language}_${locale.country}/words.txt"
        loadFromAsset(context.assets, path)
    }

    /**
     * Attempts to load an internal asset under a specified path.
     */
    override fun loadFromAsset(assetManager: AssetManager, assetPath: String) {
        unload()
        val threadCount = Runtime.getRuntime().availableProcessors()
        nativeIndex = loadNative(assetManager, assetPath, threadCount)
        if (nativeIndex.nil) {
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
            val query = Normalizer.normalize(pattern.lowercase(locale), Normalizer.Form.NFKC)
            findPartialNative(nativeIndex.getPointer(), query, cursor, limit)
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
        var current: MissingLettersIndex? = null
    }
}
