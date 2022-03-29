package xyz.lukasz.xword

import android.os.Bundle
import android.view.View
import android.view.ViewGroup
import android.view.inputmethod.InputMethodManager
import android.widget.FrameLayout
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.commit
import androidx.transition.Fade
import androidx.transition.TransitionManager
import com.google.android.material.snackbar.Snackbar
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch

@AndroidEntryPoint
class MainActivity : AppCompatActivity() {

    private lateinit var loadingFrameLayout: FrameLayout
    private lateinit var mainLayout: ViewGroup

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        loadingFrameLayout = findViewById(R.id.loading_frame_layout)
        mainLayout = findViewById(R.id.main_layout)

        val searchFragment = SearchFragment()
        supportFragmentManager.commit {
            add(R.id.fragment_container_view, searchFragment)
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
        val container = R.id.fragment_container_view
        val fragment = DefinitionFragment(word)
        supportFragmentManager.commit {
            setCustomAnimations(
                R.anim.slide_in,
                R.anim.fade_out,
                R.anim.fade_in,
                R.anim.slide_out
            )
            setReorderingAllowed(true)
            add(container, fragment)
            addToBackStack("Definition of $word")
        }

        val imm = getSystemService(INPUT_METHOD_SERVICE) as InputMethodManager
        if (imm.isAcceptingText) {
            imm.hideSoftInputFromWindow(currentFocus?.windowToken, 0)
        }
    }

    private fun switchIndexCategory(mode: String) {
        runOnUiThread {

            val fade = Fade().apply {
                duration = 150
                addTarget(loadingFrameLayout)
            }

            // Fade the overlay in
            TransitionManager.beginDelayedTransition(mainLayout, fade)
            loadingFrameLayout.visibility = View.VISIBLE

            GlobalScope.launch {
                val dict = Dictionary.current ?: Dictionary("pl", "PL")
                dict.loadFromAsset(this@MainActivity)
                Dictionary.current = dict
            }.invokeOnCompletion { cause ->
                runOnUiThread {

                    // Fade the overlay out
                    TransitionManager.beginDelayedTransition(mainLayout, fade)
                    loadingFrameLayout.visibility = View.GONE

                    // If the job has been aborted, notify the user
                    if (cause != null) {
                        val resources = mainLayout.resources
                        Snackbar.make(
                            mainLayout,
                            resources.getString(if (cause is CancellationException) {
                                R.string.operation_cancelled
                            } else {
                                R.string.operation_failed
                            }),
                            Snackbar.LENGTH_LONG)
                            .show()
                    }
                }
            }
        }
    }
}
