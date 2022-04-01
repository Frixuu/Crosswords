package xyz.lukasz.xword

import android.os.Bundle
import android.text.Editable
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import timber.log.Timber
import xyz.lukasz.xword.databinding.FragmentSearchBoxBinding
import xyz.lukasz.xword.utils.normalize
import xyz.lukasz.xword.utils.showSnackbar
import java.text.Normalizer.Form.NFKC
import java.util.*
import java.util.concurrent.atomic.AtomicReference
import kotlin.concurrent.thread

class SearchBoxFragment(
    private val parentFragment: SearchFragment
) : Fragment(R.layout.fragment_search_box), TextChangedListener {

    private var binding: FragmentSearchBoxBinding? = null
    private val mostRecentThread = AtomicReference<Thread?>(null)

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        val binding = FragmentSearchBoxBinding.inflate(inflater, container, false)
        binding.userInputEdittext.addTextChangedListener(this)
        this.binding = binding
        return binding.root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        binding?.userInputEdittext?.removeTextChangedListener(this)
        binding = null
    }

    override fun afterTextChanged(s: Editable?) {

        if (s == null) {
            Timber.w("Editable is null!")
            return
        }

        val currentDict = Dictionary.current
        if (currentDict == null) {
            Timber.w("Current dictionary is null!")
            return
        }

        if (!currentDict.ready) {
            Timber.w("Current dictionary exists, but is not loaded yet!")
            return
        }

        val pattern = s.normalize(NFKC)
        val searchThread = thread(name = "Missing letter search ($pattern)", start = false) {
            val currentThread = Thread.currentThread()
            val limit = 250
            val results = currentDict.findPartial(pattern, null, limit)
            Timber.i("Found %d results for pattern '%s'", results.size, pattern)

            // If these results are already out of date, don't bother updating the UI
            if (currentThread != mostRecentThread.get()) {
                return@thread
            }

            val cursor = results.lastOrNull()
            val resultList = mutableListOf(*results)
            Collections.sort(resultList, currentDict.collator)
            parentFragment.updateSearchResults(resultList)

            if (results.size >= limit) {
                val mainLayout: ViewGroup = requireActivity().findViewById(R.id.main_layout)
                val message = mainLayout.resources
                    .getText(R.string.search_showing_only)
                    .toString()
                mainLayout.showSnackbar(String.format(message, results.size))
            }
        }

        mostRecentThread.set(searchThread)
        searchThread.start()
    }
}
