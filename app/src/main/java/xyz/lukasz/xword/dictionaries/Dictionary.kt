package xyz.lukasz.xword.dictionaries

import android.app.Activity
import android.util.Log
import org.joda.time.*
import java.io.BufferedReader
import java.nio.CharBuffer
import java.nio.charset.Charset
import java.time.ZonedDateTime

class Dictionary {

    val words = mutableSetOf<String>()
    var forwardIndex = WordNode(null)

    companion object {
        var current: Dictionary? = null

        fun loadFromAsset(activity: Activity, filename: String): Dictionary {

            val before = Instant.now()
            val dict = Dictionary()

            val inputStream = activity.assets.open(filename)
            val reader = inputStream.reader(Charsets.UTF_8)
            val bufReader = BufferedReader(reader)

            while (true) {
                val str = bufReader.readLine() ?: break
                dict.words.add(str)
                dict.forwardIndex.pushWord(str, 0)
            }

            bufReader.close()
            val after = Instant.now()
            Log.i("Dictionary", "Loading dictionary \"$filename\"took ${after.millis - before.millis}ms")

            return dict
        }
    }
}