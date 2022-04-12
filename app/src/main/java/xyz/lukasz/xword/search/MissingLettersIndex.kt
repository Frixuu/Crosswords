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
class MissingLettersIndex(locale: Locale) : WordIndex(locale) {

    /**
     * Attempts to load an internal asset under a specified path.
     */
    override fun loadFromAsset(assetManager: AssetManager) {
        unload()
        val assetPath = resolveAssetPath()
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
