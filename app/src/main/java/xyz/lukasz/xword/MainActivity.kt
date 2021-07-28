package xyz.lukasz.xword

import android.os.Bundle
import android.os.Handler
import android.os.Looper
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
import kotlinx.coroutines.Job
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import org.joda.time.Instant
import xyz.lukasz.xword.dictionaries.Dictionary

class MainActivity : AppCompatActivity() {

    external fun fooFromNative(): Int

    private var currentJob: Job? = null
    private val currentJobLock = Any()

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
                GlobalScope.launch {
                    coroutineScope {
                        var results: List<String> = listOf()
                        var thisJob: Job? = null
                        synchronized (currentJobLock) {
                            currentJob?.cancel()
                            thisJob = launch {
                                results = currentDict.findPartial(s.toString())
                            }
                            currentJob = thisJob
                        }
                        thisJob!!.join()
                        synchronized(currentJobLock) {
                            if (thisJob == currentJob) {
                                val handler = Handler(Looper.getMainLooper())
                                handler.post {
                                    recyclerView.adapter = WordAdapter(results, recyclerView)
                                }
                            }
                        }
                    }
                }
            }
        })

        val tabLayout: TabLayout = findViewById(R.id.tabLayout)
        tabLayout.addOnTabSelectedListener(object : TabLayout.OnTabSelectedListener {

            override fun onTabSelected(tab: TabLayout.Tab?) {
                // Handle tab select
                Log.i("MA", "tab selected: ${tab?.text}")
                GlobalScope.launch {
                    val before = Instant.now()
                    val dict = Dictionary()
                    dict.loadFromAsset(this@MainActivity, "dictionaries/pl_PL/words.txt");
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