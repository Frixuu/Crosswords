package xyz.lukasz.xword

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import com.google.android.material.tabs.TabLayout
import xyz.lukasz.xword.dictionaries.Dictionary

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        val tabLayout: TabLayout = findViewById(R.id.tabLayout)
        tabLayout.addOnTabSelectedListener(object : TabLayout.OnTabSelectedListener {

            override fun onTabSelected(tab: TabLayout.Tab?) {
                // Handle tab select
                Log.i("MA", "tab selected: ${tab?.text}")
            }

            override fun onTabReselected(tab: TabLayout.Tab?) {
                Log.i("MA", "tab reselected: ${tab?.text}")
            }

            override fun onTabUnselected(tab: TabLayout.Tab?) {
                Log.i("MA", "tab unselected: ${tab?.text}")
            }
        })

        val dictionary = Dictionary.loadFromAsset(this, "dictionaries/pl_PL/words.txt")
        Dictionary.current = dictionary
    }
}