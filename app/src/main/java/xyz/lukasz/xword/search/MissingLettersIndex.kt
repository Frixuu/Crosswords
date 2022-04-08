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
     * A native method that attempts to load and index a native Dictionary
     * and returns a pointer to that object
     * or null, if the operation failed.
     */
    private external fun loadNative(
        assetManager: AssetManager,
        filename: String,
        concLevel: Int
    ): NativeSharedPointer

    companion object {
        var current: MissingLettersIndex? = null
    }
}
