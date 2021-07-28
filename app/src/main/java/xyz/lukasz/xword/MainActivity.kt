package xyz.lukasz.xword

import android.os.Bundle
import android.util.Log
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.DefaultItemAnimator
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.snackbar.Snackbar
import com.google.android.material.tabs.TabLayout
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import org.joda.time.Instant
import xyz.lukasz.xword.dictionaries.Dictionary

class MainActivity : AppCompatActivity() {

    external fun fooFromNative(): Int

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
                    recyclerView.adapter = WordAdapter(dict.findPartial(".a.ka"), recyclerView)
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