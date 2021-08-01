package xyz.lukasz.xword

import android.os.Bundle
import android.text.Editable
import android.text.TextWatcher
import android.util.Log
import android.view.View
import android.widget.EditText
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.DefaultItemAnimator
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.snackbar.Snackbar
import com.google.android.material.tabs.TabLayout
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import org.joda.time.Instant
import java.text.Collator
import java.util.*

class MainActivity : AppCompatActivity() {

    external fun fooFromNative(): Int

    private var mostRecentThread : Thread? = null
    private val currentThreadLock = Any()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val view = findViewById<View>(R.id.main_layout)
        Snackbar.make(view, "Native says: ${fooFromNative()}!", Snackbar.LENGTH_LONG).show()

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
                if (!currentDict.isLoaded()) {
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
                            Collections.sort(resultList, Collator.getInstance())
                            runOnUiThread {
                                recyclerView.adapter = WordAdapter(resultList, recyclerView)
                                if (results.size >= limit) {
                                    val message = view.resources.getText(R.string.search_showing_only).toString()
                                    Snackbar.make(view,
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
                // Handle tab select
                Log.i("MA", "tab selected: ${tab?.text}")
                GlobalScope.launch {
                    val before = Instant.now()
                    val dict = Dictionary.current ?: Dictionary()
                    dict.loadFromAsset(this@MainActivity, "dictionaries/pl_PL/words.txt")
                    Dictionary.current = dict
                    val after = Instant.now()
                    Snackbar.make(view, "Loading dictionary took ${after.millis - before.millis}ms", Snackbar.LENGTH_LONG).show()
                }
            }

            override fun onTabReselected(tab: TabLayout.Tab?) {
                Log.i("MA", "tab reselected: ${tab?.text}")
            }

            override fun onTabUnselected(tab: TabLayout.Tab?) {
                Log.i("MA", "tab unselected: ${tab?.text}")
            }
        })
    }

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }
}