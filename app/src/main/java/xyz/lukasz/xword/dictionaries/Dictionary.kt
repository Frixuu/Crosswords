package xyz.lukasz.xword.dictionaries

import android.app.Activity
import android.content.res.AssetFileDescriptor
import android.util.Log
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.joinAll
import kotlinx.coroutines.launch
import org.joda.time.*
import java.nio.ByteBuffer
import java.nio.channels.FileChannel
import kotlin.experimental.and

class Dictionary {

    val words = mutableSetOf<String>()
    var forwardIndex = WordNode(null)

    companion object {
        var current: Dictionary? = null

        private const val BYTE_LINE_FEED = '\n'.code.toByte()
        private const val BYTE_CARRIAGE_RETURN = '\r'.code.toByte()

        private const val CODEPOINT_ONE_BYTE_MASK: Byte = (0b10000000).toByte()
        private const val CODEPOINT_TWO_BYTES_MASK: Byte = (0b11000000).toByte()
        private const val CODEPOINT_THREE_BYTES_MASK: Byte = (0b11100000).toByte()
        private const val CODEPOINT_FOUR_BYTES_MASK: Byte = (0b11110000).toByte()

        private val newLineBytes = arrayOf(BYTE_LINE_FEED, BYTE_CARRIAGE_RETURN)

        suspend fun loadFromAsset(activity: Activity, filename: String): Dictionary {

            val before = Instant.now()
            val dict = Dictionary()

            val desc = activity.assets.openFd(filename)
            val length = desc.declaredLength
            if (length == AssetFileDescriptor.UNKNOWN_LENGTH) {
                throw IllegalStateException("The length of the asset file $filename is unknown")
            }

            val fileChannel = desc.createInputStream().channel
            val mappedBuffer = fileChannel!!.map(FileChannel.MapMode.READ_ONLY, 0, length)
            mappedBuffer.load()
            Log.i("Dictionary", "Mmaped asset $filename of length $length")

            val concurrentCount: Int = Runtime.getRuntime().availableProcessors()
            Log.i("Dictionary", "Reading mmaped file in $concurrentCount coroutines")

            val startIndices = IntArray(concurrentCount)
            startIndices[0] = 0
            (1 until concurrentCount).forEach { i ->
                var indexCandidate = (i * length / concurrentCount).toInt()
                while (indexCandidate < length) {
                    val thisByte = mappedBuffer[indexCandidate++]
                    val nextByte = mappedBuffer[indexCandidate]
                    if (thisByte in newLineBytes && nextByte !in newLineBytes) {
                        startIndices[i] = indexCandidate + 1
                        break;
                    }
                }
            }
            Log.i("Dictionary", "Starting to scan the file from indices ${startIndices.joinToString(", ", "[", "]")}")

            val parsingJobs = (0 until concurrentCount).map { i ->
                val startIndex = startIndices[i]
                val endIndex = if (concurrentCount == startIndices.size) {
                    length.toInt()
                } else {
                    startIndices[i + 1]
                }

                coroutineScope {
                    launch {
                        var index = startIndex
                        var bytesTillCodepointEnd = 0
                        val buffer = ByteArray(256)
                        var bufferIndex = 0
                        while (index < endIndex) {
                            val byte = mappedBuffer[index++]
                            if (bytesTillCodepointEnd != 0) {
                                bytesTillCodepointEnd--
                                buffer[bufferIndex++] = byte
                            } else {
                                if (byte != BYTE_LINE_FEED && byte != BYTE_CARRIAGE_RETURN) {
                                    buffer[bufferIndex++] = byte
                                    if ((byte and CODEPOINT_ONE_BYTE_MASK) != CODEPOINT_ONE_BYTE_MASK) {
                                        continue;
                                    } else if ((byte and CODEPOINT_TWO_BYTES_MASK) == CODEPOINT_TWO_BYTES_MASK) {
                                        bytesTillCodepointEnd = 1
                                    } else if ((byte and CODEPOINT_THREE_BYTES_MASK) == CODEPOINT_THREE_BYTES_MASK) {
                                        bytesTillCodepointEnd = 1
                                    } else if ((byte and CODEPOINT_FOUR_BYTES_MASK) == CODEPOINT_FOUR_BYTES_MASK) {
                                        bytesTillCodepointEnd = 1
                                    }
                                } else {
                                    if (bufferIndex > 0) {
                                        val word = String(buffer, 0, bufferIndex, Charsets.UTF_8)
                                        dict.words.add(word)
                                        dict.forwardIndex.pushWord(word, 0)
                                        bufferIndex = 0
                                    }
                                }
                            }
                        }

                        // If the file does not end with new line, flush the buffer
                        if (bufferIndex > 0) {
                            val word = String(buffer, 0, bufferIndex, Charsets.UTF_8)
                            dict.words.add(word)
                            dict.forwardIndex.pushWord(word, 0)
                        }
                    }
                }
            }

            parsingJobs.joinAll()
            val afterJoin = Instant.now()
            Log.i("Dictionary", "Loading dictionary \"$filename\" took ${afterJoin.millis - before.millis}ms")
            dict.forwardIndex.collectLocks(2)
            val afterUnlock = Instant.now()
            Log.i("Dictionary", "Collecting spinlocks \"$filename\" took ${afterUnlock.millis - afterJoin.millis}ms")

            return dict
        }
    }
}