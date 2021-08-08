package xyz.lukasz.xword

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import xyz.lukasz.xword.databinding.FragmentWordDefinitionBinding

class DefinitionFragment(private val word: String) : Fragment(R.layout.fragment_word_definition) {

    private var binding: FragmentWordDefinitionBinding? = null

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        binding = FragmentWordDefinitionBinding.inflate(inflater, container, false)
        binding?.word = word
        return binding?.root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        binding = null
    }
}