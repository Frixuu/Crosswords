package xyz.lukasz.xword.util

import org.joda.time.Duration
import org.joda.time.Instant

inline fun time(block: () -> Unit): Duration {
    val before = Instant.now()
    block.invoke()
    val after = Instant.now()
    return Duration(before, after)
}