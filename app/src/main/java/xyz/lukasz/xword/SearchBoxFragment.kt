package xyz.lukasz.xword

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.fragment.app.activityViewModels
import dagger.hilt.android.AndroidEntryPoint
import xyz.lukasz.xword.databinding.FragmentSearchBoxBinding
import xyz.lukasz.xword.search.SearchResultsViewModel

@AndroidEntryPoint
class SearchBoxFragment : Fragment(R.layout.fragment_search_box) {

    private lateinit var binding: FragmentSearchBoxBinding

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        binding = FragmentSearchBoxBinding.inflate(inflater, container, false)
        binding.lifecycleOwner = this
        binding.viewModel = activityViewModels<SearchResultsViewModel>().value
        return binding.root
    }
}
