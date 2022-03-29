package xyz.lukasz.xword.definitions

import org.jsoup.Jsoup

/**
 * Fetches definitions of Polish words from sjp.pl.
 */
class SjpDefinitionProvider : LocaleSpecificDefinitionProvider("pl", "PL") {

    override fun getDefinitions(word: String): List<Definition> {
        val url = "https://sjp.pl/$word"
        val connection = Jsoup.connect(url)
        val document = connection.get()

        return document.select("div > table.wtab")
            .flatMap { element ->
                val parent = element.parent()
                val definedWord = parent?.previousElementSibling()?.text() ?: word
                val definitions = parent?.nextElementSibling()?.html()?.split("<br>")
                definitions?.map {
                    var definition = it
                    // Some entries start with a number, trim it
                    val match = startsWithNumberRegex.find(definition, 0)
                    if (match != null) {
                        definition = definition.substring(match.range.last + 1)
                    }
                    definition = definition.replace("&nbsp;", " ")
                    definition = definition.trimEnd(';')
                    Definition(definedWord, definition, locale)
                } ?: emptyList()
            }
            .toList()
    }

    companion object {
        @JvmStatic private val startsWithNumberRegex: Regex = Regex("^\\d+\\. ")
    }
}
