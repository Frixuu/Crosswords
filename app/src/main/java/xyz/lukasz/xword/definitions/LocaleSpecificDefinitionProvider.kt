package xyz.lukasz.xword.definitions

import java.util.*

/**
 * An abstract class that provides definitions only for a specific locale.
 */
abstract class LocaleSpecificDefinitionProvider(val locale: Locale) : DefinitionProvider {
    constructor(lang: String, country: String) : this(Locale(lang, country))
}
