package xyz.lukasz.xword.search

import android.content.res.AssetManager
import org.jetbrains.annotations.Contract
import xyz.lukasz.xword.interop.NativeSharedPointer
import java.text.Collator
import java.text.Normalizer
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
    abstract fun loadFromAsset(assetManager: AssetManager)

    protected fun resolveAssetPath(): String {
        return "dictionaries/${locale.language}_${locale.country}/words.txt"
    }

    @Contract("_ -> new", pure = true)
    fun lookup(query: String, maxResults: Int): MutableList<String> {
        return if (ready) {
            val queryStr = Normalizer.normalize(query.lowercase(locale), Normalizer.Form.NFKC)
            val resultArray = lookupNative(nativeIndex.getPointer(), queryStr, maxResults)
            mutableListOf(*resultArray)
        } else {
            mutableListOf()
        }
    }

    private external fun lookupNative(pointer: Long, query: String, max: Int): Array<String>

    open fun unload() {
        nativeIndex.free()
    }
}
