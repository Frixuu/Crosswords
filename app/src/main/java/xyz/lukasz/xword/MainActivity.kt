package xyz.lukasz.xword

import android.os.Bundle
import android.view.View
import android.view.inputmethod.InputMethodManager
import androidx.appcompat.app.AppCompatActivity
import androidx.databinding.DataBindingUtil
import androidx.fragment.app.commit
import androidx.lifecycle.lifecycleScope
import androidx.transition.Fade
import androidx.transition.TransitionManager
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.coroutines.*
import xyz.lukasz.xword.databinding.ActivityMainBinding
import xyz.lukasz.xword.utils.showSnackbar
import javax.inject.Inject

@AndroidEntryPoint
class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    @Inject lateinit var imm: InputMethodManager

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = DataBindingUtil.setContentView(this, R.layout.activity_main)

        val searchFragment = SearchFragment()
        supportFragmentManager.commit {
            add(binding.fragmentContainerView.id, searchFragment, "search")
        }

        /*
        val tabLayout: TabLayout = findViewById(R.id.tabLayout)
        tabLayout.addOnTabSelectedListener(object : TabLayout.OnTabSelectedListener {

            override fun onTabSelected(tab: TabLayout.Tab?) {
                Log.i("MainActivity", "tab selected: ${tab?.text}")
                switchIndexCategory("unused")
            }

            override fun onTabReselected(tab: TabLayout.Tab?) {
                Log.i("MainActivity", "tab reselected: ${tab?.text}")
            }

            override fun onTabUnselected(tab: TabLayout.Tab?) {
                Log.i("MainActivity", "tab unselected: ${tab?.text}")
            }
        })

         */

        switchIndexCategory("unused")
    }

    /**
     * Pushes a fragment containing a word's definition.
     * @param word Model of the word to display.
     */
    fun showWordDefinition(word: String) {
        supportFragmentManager.commit {
            setCustomAnimations(
                R.anim.slide_in,
                R.anim.fade_out,
                R.anim.fade_in,
                R.anim.slide_out
            )
            add(binding.fragmentContainerView.id, DefinitionFragment(word))
            addToBackStack("Definition of $word")
            setReorderingAllowed(true)
        }

        hideSoftwareKeyboard()
    }

    /**
     * If some focused element is accepting text,
     * blurs it and hides the software keyboard.
     */
    private fun hideSoftwareKeyboard() {
        if (imm.isAcceptingText) {
            val focus = currentFocus ?: return
            imm.hideSoftInputFromWindow(focus.windowToken, 0)
            focus.clearFocus()
        }
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
