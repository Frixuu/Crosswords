package xyz.lukasz.xword

import android.os.Bundle
import android.text.Editable
import android.text.TextWatcher
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.inputmethod.EditorInfo
import androidx.fragment.app.Fragment
import com.google.android.material.snackbar.Snackbar
import xyz.lukasz.xword.databinding.FragmentSearchBoxBinding
import java.util.*

class SearchBoxFragment(
    private val parentFragment: SearchFragment
) : Fragment(R.layout.fragment_search_box), TextWatcher {

    private var binding: FragmentSearchBoxBinding? = null

    private var mostRecentThread: Thread? = null
    private val currentThreadLock = Any()

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        binding = FragmentSearchBoxBinding.inflate(inflater, container, false)
        binding?.userInputEdittext?.apply {
            addTextChangedListener(this@SearchBoxFragment)
            imeOptions = EditorInfo.IME_ACTION_NONE
        }
        return binding?.root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        binding?.userInputEdittext?.removeTextChangedListener(this)
        binding = null
    }

    override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {

    }

    override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {

    }

    override fun afterTextChanged(s: Editable?) {

        if (s == null) {
            Log.w("SearchBoxFragment", "Editable is null!")
            return
        }

        val currentDict = Dictionary.current
        if (currentDict == null) {
            Log.w("SearchBoxFragment", "Current dictionary is null!")
            return
        }

        if (!currentDict.loaded) {
            Log.w("SearchBoxFragment", "Current dictionary exists, but is not loaded yet!")
            return
        }

        val searchThread = Thread {
            val currentThread = Thread.currentThread().apply { name = "Word search" }
            val limit = 250
            val results = currentDict.findPartial(s.toString(), null, limit)
            Log.i("SearchBoxFragment", "Found ${results.size} results")
            synchronized(currentThreadLock) {
                if (currentThread == mostRecentThread) {
                    val cursor = results.lastOrNull()
                    val resultList = mutableListOf(*results)
                    Collections.sort(resultList, currentDict.collator)
                    activity?.run {
                        runOnUiThread {
                            parentFragment.recyclerView?.adapter =
                                SingleWordAdapter(resultList, this as MainActivity)
                            if (results.size >= limit) {
                                val mainLayout: ViewGroup = findViewById(R.id.main_layout)
                                val resources = mainLayout.resources
                                val message =
                                    resources.getText(R.string.search_showing_only).toString()
                                Snackbar.make(
                                    mainLayout,
                                    String.format(message, results.size),
                                    Snackbar.LENGTH_LONG
                                )
                                    .show()
                            }
                        }
                    }
                }
            }
        }

        synchronized(currentThreadLock) {
            mostRecentThread = searchThread
        }

        searchThread.start()
    }
}
