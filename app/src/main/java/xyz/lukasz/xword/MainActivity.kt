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
import xyz.lukasz.xword.search.SearchResultsViewModel
import xyz.lukasz.xword.utils.showSnackbar
import java.util.*

@AndroidEntryPoint
class MainActivity : ActivityBase<ActivityMainBinding>(R.layout.activity_main) {

    private val resultsViewModel: SearchResultsViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

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

        switchIndexCategory("unused")
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

    private fun switchIndexCategory(mode: String) {
        runOnUiThread {

            val fade = Fade().apply {
                duration = 150
                addTarget(binding.loadingFrameLayout)
            }

            // Fade the overlay in
            TransitionManager.beginDelayedTransition(binding.mainLayout, fade)
            binding.loadingFrameLayout.visibility = View.VISIBLE

            val dict = Dictionary.current ?: Dictionary("pl", "PL")
            lifecycleScope.launch(Dispatchers.IO) {
                dict.loadFromAsset(this@MainActivity)
                Dictionary.current = dict
                resultsViewModel.index.postValue(dict)
            }.invokeOnCompletion { cause ->
                runOnUiThread {
                    // Fade the overlay out
                    TransitionManager.beginDelayedTransition(binding.mainLayout, fade)
                    binding.loadingFrameLayout.visibility = View.GONE

                    // If the job has been aborted, notify the user
                    val layout = binding.mainLayout
                    when (cause) {
                        null -> { }
                        is CancellationException -> layout.showSnackbar(R.string.operation_cancelled)
                        else -> layout.showSnackbar(R.string.operation_failed)
                    }
                }
            }
        }
    }
}
