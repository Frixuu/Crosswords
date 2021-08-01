package xyz.lukasz.xword

import android.app.Activity
import android.content.res.AssetManager
import java.util.*

class Dictionary {

    private var nativePtr: Long = 0

    private external fun loadNative(assetManager: AssetManager,
                                    filename: String,
                                    oldPtr: Long,
                                    concLevel: Int): Long

    private external fun findPartialNative(nativePtr: Long,
                                           word: String,
                                           cursor: String?,
                                           limit: Int): Array<String>

    fun isLoaded(): Boolean {
        return nativePtr != 0L
    }

    fun loadFromAsset(activity: Activity, filename: String) {
        val assetManager = activity.assets
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
        assert(limit <= 500) { "Limit too high, possible JNI reference table overflow" }
        return if (isLoaded()) {
            findPartialNative(nativePtr, pattern.lowercase(), cursor, limit)
        } else {
            emptyArray()
        }
    }

    companion object {
        var current: Dictionary? = null
    }
}