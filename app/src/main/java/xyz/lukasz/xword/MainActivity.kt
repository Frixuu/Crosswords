package xyz.lukasz.xword

import android.app.ActivityOptions
import android.os.Bundle
import android.view.View
import androidx.activity.viewModels
import androidx.fragment.app.commit
import androidx.lifecycle.lifecycleScope
import androidx.transition.Fade
import androidx.transition.TransitionManager
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import xyz.lukasz.xword.databinding.ActivityMainBinding
import xyz.lukasz.xword.definitions.DefineIntent
import xyz.lukasz.xword.search.MissingLettersIndex
import xyz.lukasz.xword.search.SearchResultsViewModel
import xyz.lukasz.xword.search.WordIndexFactory
import xyz.lukasz.xword.search.WordIndexType
import xyz.lukasz.xword.utils.showSnackbar
import java.util.*

@AndroidEntryPoint
class MainActivity : ActivityBase<ActivityMainBinding>(R.layout.activity_main) {

    private val resultsViewModel: SearchResultsViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        resultsViewModel.index.observe(this) {
            // Animate the overlay
            TransitionManager.beginDelayedTransition(binding.mainLayout, Fade().apply {
                duration = 150; addTarget(binding.loadingFrameLayout)
            })

            if (it == null) {
                binding.loadingFrameLayout.visibility = View.VISIBLE
            } else {
                binding.loadingFrameLayout.visibility = View.GONE
            }
        }

        resultsViewModel.results.observe(this) {
            val maxResults = SearchResultsViewModel.MAX_RESULTS
            if (it.size >= maxResults) {
                val message = resources.getString(R.string.search_showing_only, it.size)
                binding.mainLayout.showSnackbar(message)
            }
        }

        val searchFragment = SearchFragment()
        supportFragmentManager.commit {
            add(binding.fragmentContainerView.id, searchFragment, "search")
        }

        resultsViewModel.switchIndexCategory(this.assets, WordIndexType.MISSING_LETTERS)
    }

    /**
     * Pushes a fragment containing a word's definition.
     * @param word Model of the word to display.
     */
    fun showWordDefinition(originView: View, textView: View, word: String, locale: Locale) {
        val intent = DefineIntent(this, word, locale)
        val options = ActivityOptions
            .makeSceneTransitionAnimation(this)
            .toBundle()

        startActivity(intent, options)
    }
}
