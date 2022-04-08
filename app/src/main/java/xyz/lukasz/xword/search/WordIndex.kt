package xyz.lukasz.xword.search

import android.content.res.AssetManager
import xyz.lukasz.xword.interop.NativeSharedPointer
import java.text.Collator
import java.util.*

abstract class WordIndex(
    /**
     * Locale of this index.
     * Words returned from this index will be in this language.
     */
    val locale: Locale
) {

    /**
     * Collator for this index.
     * It will be used for comparing and sorting strings returned from this index.
     */
    val collator: Collator = Collator.getInstance(locale)

    /**
     * Pointer to a native index object.
     */
    protected var nativeIndex = NativeSharedPointer.nil()

    /**
     * Is this index in a valid state?
     */
    val ready: Boolean
        get() = !nativeIndex.nil

    /**
     * Loads contents of this index from a raw asset file.
     */
    abstract fun loadFromAsset(assetManager: AssetManager, assetPath: String)

    open fun unload() {
        nativeIndex.free()
    }
}
