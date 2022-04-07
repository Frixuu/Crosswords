package xyz.lukasz.xword.search

import androidx.lifecycle.*
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import timber.log.Timber
import xyz.lukasz.xword.Dictionary
import java.text.Normalizer
import java.util.*
import javax.inject.Inject

@HiltViewModel
class SearchResultsViewModel @Inject constructor(
    private val savedState: SavedStateHandle
) : ViewModel() {

    private val _results = MutableLiveData<List<String>>()
    val results: LiveData<List<String>> get() = _results

    private var index: Dictionary? = null
    private var query: String = savedState.get<String>("query") ?: ""

    fun setIndex(newIndex: Dictionary) {
        this.index = newIndex
        if (newIndex.ready) {
            searchAndUpdateResults(newIndex, query, MAX_RESULTS)
        }
    }

    fun setQuery(newQuery: String) {
        this.query = newQuery
        savedState.set("query", newQuery)
        if (index?.ready == true) {
            searchAndUpdateResults(index!!, newQuery, MAX_RESULTS)
        }
    }

    private fun searchAndUpdateResults(index: Dictionary, query: String, maxResults: Int) {
        val input = Normalizer.normalize(query, Normalizer.Form.NFKC)
        viewModelScope.launch(Dispatchers.IO) {
            val queryResults = index.findPartial(input, null, maxResults)
            Timber.i("Found %d results for pattern '%s'", queryResults.size, input)
            Arrays.sort(queryResults, index.collator)
            _results.postValue(queryResults.asList())
        }
    }

    companion object {
        const val MAX_RESULTS: Int = 500
    }
}

