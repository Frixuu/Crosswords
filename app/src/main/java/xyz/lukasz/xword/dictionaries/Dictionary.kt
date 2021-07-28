package xyz.lukasz.xword.dictionaries

import android.app.Activity
import android.content.res.AssetManager

class Dictionary {

    private var nativePtr: Long = 0

    private external fun loadNative(assetManager: AssetManager, filename: String, oldPtr: Long, concLevel: Int): Long
    private external fun findPartialNative(nativePtr: Long, word: String): Array<String>

    fun isLoaded(): Boolean {
        return nativePtr != 0L
    }

    suspend fun loadFromAsset(activity: Activity, filename: String) {
        val assetManager = activity.assets
        val oldPtr = nativePtr
        nativePtr = 0
        val ptr = loadNative(assetManager, filename, oldPtr, 4)
        if (ptr == 0L) {
            throw Exception("Native loading failed")
        }
        nativePtr = ptr
    }

    /**
     * Finds all matching a provided pattern.
     * For example, a pattern ".ró." would match "drób", "próg" and "król".
     */
    suspend fun findPartial(pattern: String): List<String> {
        return if (isLoaded()) findPartialNative(nativePtr, pattern.lowercase()).toList() else listOf()
    }

    companion object {
        var current: Dictionary? = null
    }
}