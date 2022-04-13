package xyz.lukasz.xword.search

import android.content.res.AssetManager
import xyz.lukasz.xword.interop.NativeSharedPointer
import java.util.*

class AnagramIndex(locale: Locale) : WordIndex(locale) {

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
    private external fun loadNative(assetManager: AssetManager, filename: String, threads: Int)
        : NativeSharedPointer
}
