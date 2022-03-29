package xyz.lukasz.xword

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import timber.log.Timber
import xyz.lukasz.xword.databinding.FragmentWordDefinitionBinding
import xyz.lukasz.xword.definitions.DefinitionProvider
import java.util.*
import javax.inject.Inject

/**
 * Fragment that displays the definition of the word.
 */
@AndroidEntryPoint
class DefinitionFragment(private val word: String) : Fragment(R.layout.fragment_word_definition) {

    private var binding: FragmentWordDefinitionBinding? = null
    @Inject lateinit var definitionProvider: DefinitionProvider

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        val binding = FragmentWordDefinitionBinding.inflate(inflater, container, false)
        binding.word = word

        val locale = Dictionary.current?.locale ?: Locale("pl", "PL")
        tryFetchDefinitionsAndUpdate(word, locale)

        this.binding = binding
        return binding.root
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
            try {

                val definitions = withContext(Dispatchers.IO) {
                    definitionProvider.getDefinitions(word)
                        .filter { it.locale == locale }
                }

                Timber.tag("DefinitionFragment")
                    .i("Fetched %d definitions for word '%s' in locale %s",
                        definitions.size, word, locale.displayLanguage)

                val definitionText = definitions
                    .joinToString(separator = "\n") {
                        if (it.word != word) {
                            " - (${it.word}) ${it.definition}"
                        } else {
                            " - ${it.definition}"
                        }
                    }

                binding?.definitionSjp = definitionText

            } catch (e: Exception) {
                Timber.e(e, "Error fetching definition")
                binding?.definitionSjp = "[error]"
            }
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        binding = null
    }
}
