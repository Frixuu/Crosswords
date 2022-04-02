package xyz.lukasz.xword

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.fragment.app.commit
import androidx.recyclerview.widget.DefaultItemAnimator
import xyz.lukasz.xword.databinding.FragmentSearchBinding

class SearchFragment: Fragment(R.layout.fragment_search) {

    private lateinit var binding: FragmentSearchBinding

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        binding = FragmentSearchBinding.inflate(inflater, container, false)
        binding.recyclerView.apply {
            setHasFixedSize(true)
            itemAnimator = DefaultItemAnimator().apply {
                changeDuration = 33L
                addDuration = 33L
                removeDuration = 33L
                moveDuration = 33L
            }
            adapter = SingleWordAdapter(requireActivity() as MainActivity).apply {
                setHasStableIds(false)
                submitList(emptyList())
            }
        }

        val searchBoxFragment = SearchBoxFragment(this)
        activity?.supportFragmentManager?.commit {
            add(R.id.search_box_fragment_container, searchBoxFragment)
        }

        return binding.root
    }

    /**
     * This method populates the result view with the given list of words.
     * It is safe to call from any thread.
     */
    fun updateSearchResults(results: List<String>) {
        this.requireView().post {
            val adapter = binding.recyclerView.adapter as SingleWordAdapter
            adapter.submitList(results)
        }
    }
}
