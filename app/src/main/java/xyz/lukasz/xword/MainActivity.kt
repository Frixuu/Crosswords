package xyz.lukasz.xword

import android.os.Bundle
import android.text.Editable
import android.text.TextWatcher
import android.util.Log
import android.view.View
import android.view.ViewGroup
import android.widget.EditText
import android.widget.FrameLayout
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.DefaultItemAnimator
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import androidx.transition.Fade
import androidx.transition.TransitionManager
import com.google.android.material.snackbar.Snackbar
import com.google.android.material.tabs.TabLayout
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import xyz.lukasz.xword.util.time
import java.util.*

class MainActivity : AppCompatActivity() {

    private lateinit var loadingFrameLayout: FrameLayout
    private lateinit var mainLayout: ViewGroup

    private var mostRecentThread : Thread? = null
    private val currentThreadLock = Any()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        loadingFrameLayout = findViewById(R.id.loading_frame_layout)
        mainLayout = findViewById(R.id.main_layout)

        val recyclerView = findViewById<RecyclerView>(R.id.recycler_view).apply {
            setHasFixedSize(true)
            layoutManager = LinearLayoutManager(this@MainActivity)
            itemAnimator = DefaultItemAnimator()
        }

        val editText: EditText = findViewById(R.id.user_input_edittext)
        editText.addTextChangedListener(object : TextWatcher {
            override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {

            }

            override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {

            }

            override fun afterTextChanged(s: Editable?) {
                val currentDict = Dictionary.current
                if (currentDict == null) {
                    Log.w("MainActivity", "Current dictionary is null!")
                    return
                }
                if (!currentDict.loaded) {
                    Log.w("MainActivity", "Current dictionary exists, but is not loaded yet!")
                    return
                }
                if (s == null) {
                    Log.w("MainActivity", "Editable is null!")
                    return
                }

                val searchThread = Thread {
                    val currentThread = Thread.currentThread().apply { name = "Word search" }
                    val limit = 200
                    val results = currentDict.findPartial(s.toString(), null, limit)
                    synchronized (currentThreadLock) {
                        if (currentThread == mostRecentThread) {
                            val cursor = results.lastOrNull()
                            val resultList = mutableListOf(*results)
                            Collections.sort(resultList, currentDict.collator)
                            runOnUiThread {
                                recyclerView.adapter = WordAdapter(resultList, recyclerView)
                                if (results.size >= limit) {
                                    val resources = mainLayout.resources
                                    val message = resources.getText(R.string.search_showing_only).toString()
                                    Snackbar.make(mainLayout,
                                        String.format(message, results.size),
                                        Snackbar.LENGTH_LONG)
                                        .show()
                                }
                            }
                        }
                    }
                }

                synchronized (currentThreadLock) {
                    mostRecentThread = searchThread
                }

                searchThread.start()
            }
        })

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

        switchIndexCategory("unused")
    }

    private fun switchIndexCategory(mode: String) {
        val fadeDuration: Long = 150
        runOnUiThread {
            // Fade the overlay in
            TransitionManager.beginDelayedTransition(mainLayout, Fade().apply {
                duration = fadeDuration
                addTarget(loadingFrameLayout)
            })
            loadingFrameLayout.visibility = View.VISIBLE

            GlobalScope.launch {

                // Manipulate the dictionary
                val dict = Dictionary.current ?: Dictionary("pl", "PL")
                dict.loadFromAsset(this@MainActivity)
                Dictionary.current = dict

                runOnUiThread {
                    // Fade the overlay out
                    TransitionManager.beginDelayedTransition(mainLayout, Fade().apply {
                        duration = fadeDuration
                        addTarget(loadingFrameLayout)
                    })
                    loadingFrameLayout.visibility = View.GONE
                }
            }
        }
    }
}