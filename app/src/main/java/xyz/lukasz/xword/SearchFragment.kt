package xyz.lukasz.xword

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.fragment.app.activityViewModels
import androidx.fragment.app.commit
import com.google.android.material.tabs.TabLayout
import dagger.hilt.android.AndroidEntryPoint
import timber.log.Timber
import xyz.lukasz.xword.databinding.FragmentSearchBinding
import xyz.lukasz.xword.search.SearchResultsViewModel
import xyz.lukasz.xword.search.WordIndexType
import xyz.lukasz.xword.utils.Animators

@AndroidEntryPoint
class SearchFragment : Fragment(R.layout.fragment_search) {

    private lateinit var binding: FragmentSearchBinding
    private val searchResultsViewModel: SearchResultsViewModel by activityViewModels()

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        binding = FragmentSearchBinding.inflate(inflater, container, false)
        binding.lifecycleOwner = this
        binding.recyclerView.apply {
            setHasFixedSize(true)
            itemAnimator = Animators.defaultWithDuration(33L)
            adapter = SingleWordAdapter(requireActivity() as MainActivity).apply {
                setHasStableIds(false)
                submitList(emptyList())
                searchResultsViewModel.results.observe(viewLifecycleOwner) {
                    submitList(it)
                }
            }
        }

        val searchBoxFragment = SearchBoxFragment()
        activity?.supportFragmentManager?.commit {
            add(R.id.search_box_fragment_container, searchBoxFragment)
        }

        val tabLayout = binding.tabLayout
        tabLayout.addOnTabSelectedListener(object : TabLayout.OnTabSelectedListener {

            override fun onTabSelected(tab: TabLayout.Tab?) {
                Timber.i("Tab selected: %s", tab?.text ?: "null")
                val position = tab?.position ?: return
                val mode = arrayOf(
                    WordIndexType.MISSING_LETTERS,
                    WordIndexType.ANAGRAMS
                ).getOrNull(position) ?: return
                searchResultsViewModel.switchIndexCategory(resources.assets, mode)
            }

            override fun onTabReselected(tab: TabLayout.Tab?) {
                Timber.i("Tab reselected: %s", tab?.text ?: "null")
            }

            override fun onTabUnselected(tab: TabLayout.Tab?) {
                Timber.i("Tab unselected: %s", tab?.text ?: "null")
            }
        })

        return binding.root
    }

    /**
     * This method populates the result view with the given list of words.
     * It is safe to call from any thread.
     */
    fun updateSearchResults(results: List<String>) {
        val adapter = binding.recyclerView.adapter as SingleWordAdapter
        adapter.submitList(results)
    }
}
