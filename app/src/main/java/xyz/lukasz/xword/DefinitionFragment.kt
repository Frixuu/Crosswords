package xyz.lukasz.xword

import android.app.appsearch.GlobalSearchSession
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import org.jsoup.Jsoup
import xyz.lukasz.xword.databinding.FragmentWordDefinitionBinding

/**
 * Fragment that displays the definition of the word.
 */
class DefinitionFragment(private val word: String) : Fragment(R.layout.fragment_word_definition) {

    private var binding: FragmentWordDefinitionBinding? = null

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
                val document = Jsoup.connect("https://sjp.pl/$word").get()
                val definition = document.select("div > table.wtab")
                    .mapNotNull { it.parent()?.nextElementSibling()?.html() }
                    .flatMap { it.split("<br>")  }
                    .map {
                        val match = Regex("^\\d+\\. ").find(it, 0)
                        if (match != null) {
                            it.substring(match.range.last + 1)
                        } else {
                            it
                        }
                    }
                    .joinToString(separator = "\n")

                activity?.runOnUiThread {
                    binding?.definitionSjp = definition
                }

            } catch (e: Exception) {
                Log.e("DefinitionFragment", "Error fetching definition", e)
                activity?.runOnUiThread {
                    binding?.definitionSjp = "[error]"
                }
            }
        }

        return binding?.root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        binding = null
    }
}
