package xyz.lukasz.xword

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.fragment.app.commit
import androidx.recyclerview.widget.DefaultItemAnimator
import androidx.recyclerview.widget.LinearLayoutManager
import xyz.lukasz.xword.databinding.FragmentSearchBinding

class SearchFragment: Fragment(R.layout.fragment_search) {

    private var binding: FragmentSearchBinding? = null

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        binding = FragmentSearchBinding.inflate(inflater, container, false)
        binding?.recyclerView?.apply {
            setHasFixedSize(true)
            layoutManager = LinearLayoutManager(activity)
            itemAnimator = DefaultItemAnimator()
        }

        val searchBoxFragment = SearchBoxFragment(this)
        activity?.supportFragmentManager?.commit {
            add(R.id.search_box_fragment_container, searchBoxFragment)
        }

        return binding?.root
    }

    /**
     * This method populates the result view with the given list of words.
     * It is safe to call from any thread.
     */
    fun updateSearchResults(results: List<String>) {
        this.requireView().post {
            val wordAdapter = SingleWordAdapter(results, requireActivity() as MainActivity)
            binding?.recyclerView?.adapter = wordAdapter
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        binding = null
    }
}
