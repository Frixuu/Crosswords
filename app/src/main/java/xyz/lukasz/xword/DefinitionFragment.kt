package xyz.lukasz.xword

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import xyz.lukasz.xword.databinding.FragmentWordDefinitionBinding
import xyz.lukasz.xword.definitions.SjpDefinitionProvider
import java.util.*

/**
 * Fragment that displays the definition of the word.
 */
class DefinitionFragment(private val word: String) : Fragment(R.layout.fragment_word_definition) {

    private var binding: FragmentWordDefinitionBinding? = null
    private val sjpProvider = SjpDefinitionProvider()

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        binding = FragmentWordDefinitionBinding.inflate(inflater, container, false)
        binding?.word = word

        // viewLifeCycleOwner.lifeCycleScope throws NetworkOnMainThreadException
        GlobalScope.launch {
            try {
                val locale = Dictionary.current?.locale ?: Locale("pl", "PL")
                val definition = sjpProvider.getDefinitions(word)
                    .filter { it.locale == locale }
                    .joinToString(separator = "\n") {
                        if (it.word != word) {
                            " - (${it.word}) ${it.definition}"
                        } else {
                            " - ${it.definition}"
                        }
                    }

                binding?.definitionSjp = definition

            } catch (e: Exception) {
                Log.e("DefinitionFragment", "Error fetching definition", e)
                binding?.definitionSjp = "[error]"
            }
        }

        return binding?.root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        binding = null
    }
}
