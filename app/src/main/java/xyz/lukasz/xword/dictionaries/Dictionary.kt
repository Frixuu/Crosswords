package xyz.lukasz.xword.dictionaries

import android.app.Activity
import android.content.res.AssetManager

class Dictionary {

    private var nativePtr: Long = 0

    private external fun loadNative(assetManager: AssetManager, filename: String, oldPtr: Long, concLevel: Int): Long
    private external fun findNative(nativePtr: Long, word: String): Boolean

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

    companion object {
        var current: Dictionary? = null
    }
}