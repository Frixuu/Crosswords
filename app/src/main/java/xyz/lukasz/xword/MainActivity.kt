package xyz.lukasz.xword

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.View
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

        val tabLayout: TabLayout = findViewById(R.id.tabLayout)
        tabLayout.addOnTabSelectedListener(object : TabLayout.OnTabSelectedListener {

            override fun onTabSelected(tab: TabLayout.Tab?) {
                // Handle tab select
                Log.i("MA", "tab selected: ${tab?.text}")
                GlobalScope.launch {
                    Dictionary.current = null
                    val before = Instant.now()
                    val dictionary = Dictionary.loadFromAsset(this@MainActivity, "dictionaries/pl_PL/words.txt")
                    Dictionary.current = dictionary
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