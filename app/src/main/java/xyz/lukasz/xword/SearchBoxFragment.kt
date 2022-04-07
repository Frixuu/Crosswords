package xyz.lukasz.xword

import android.os.Bundle
import android.text.Editable
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.fragment.app.activityViewModels
import dagger.hilt.android.AndroidEntryPoint
import timber.log.Timber
import xyz.lukasz.xword.databinding.FragmentSearchBoxBinding
import xyz.lukasz.xword.search.SearchResultsViewModel
import xyz.lukasz.xword.utils.observeOnce
import xyz.lukasz.xword.utils.showSnackbar

@AndroidEntryPoint
class SearchBoxFragment : Fragment(R.layout.fragment_search_box), TextChangedListener {

    private lateinit var binding: FragmentSearchBoxBinding

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        binding = FragmentSearchBoxBinding.inflate(inflater, container, false)
        binding.userInputEdittext.addTextChangedListener(this)
        return binding.root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        binding.userInputEdittext.removeTextChangedListener(this)
    }

    override fun afterTextChanged(s: Editable?) {

        if (s == null) {
            Timber.w("Editable is null!")
            return
        }

        val index = Dictionary.current
        if (index == null) {
            Timber.w("Current index is null!")
            return
        }

        if (!index.ready) {
            Timber.w("Current index exists, but is not loaded yet!")
            return
        }

        val resultsViewModel: SearchResultsViewModel by activityViewModels()
        resultsViewModel.setQuery(s.toString())
    }
}
