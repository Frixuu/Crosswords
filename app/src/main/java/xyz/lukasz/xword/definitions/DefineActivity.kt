package xyz.lukasz.xword.definitions

import android.os.Bundle
import androidx.lifecycle.lifecycleScope
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import timber.log.Timber
import xyz.lukasz.xword.ActivityBase
import xyz.lukasz.xword.R
import xyz.lukasz.xword.databinding.ActivityDefineBinding
import java.util.*
import javax.inject.Inject

@AndroidEntryPoint
class DefineActivity : ActivityBase<ActivityDefineBinding>(R.layout.activity_define) {

    @Inject lateinit var definitionProvider: DefinitionProvider

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val word = intent.getStringExtra(DefineIntent.EXTRA_WORD)!!
        val locale = intent.getSerializableExtra(DefineIntent.EXTRA_LOCALE) as Locale
        binding.word = word
        tryFetchDefinitionsAndUpdate(word, locale)
    }

    /**
     * This method uses a [DefinitionProvider] to fetch definitions of the word
     * and then updates the UI with the results accordingly.
     * This method is safe to use from any thread.
     * @param word The word to fetch definitions for.
     * @param locale The locale to use for fetching definitions.
     */
    private fun tryFetchDefinitionsAndUpdate(word: String, locale: Locale) {
        lifecycleScope.launch {
            var definitions: List<Definition> = emptyList()
            try {
                definitions = withContext(Dispatchers.IO) {
                    definitionProvider.getDefinitions(word).filter { it.locale == locale }
                }
            } catch (e: Exception) {
                Timber.e(e, "Error fetching definition")
                binding.definitionSjp = "[error]"
            }

            Timber.i("Fetched %d definitions for word '%s' in locale %s",
                definitions.size, word, locale.displayLanguage)

            binding.definitionSjp = definitions
                .joinToString(separator = "\n") {
                    if (it.word != word) {
                        " - (${it.word}) ${it.definition}"
                    } else {
                        " - ${it.definition}"
                    }
                }
        }
    }
}
