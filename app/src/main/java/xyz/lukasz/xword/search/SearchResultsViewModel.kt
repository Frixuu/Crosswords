package xyz.lukasz.xword.search

import androidx.annotation.AnyThread
import androidx.lifecycle.*
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import timber.log.Timber
import java.text.Normalizer
import java.util.*
import javax.inject.Inject

@HiltViewModel
class SearchResultsViewModel @Inject constructor(
    private val savedState: SavedStateHandle
) : ViewModel() {

    private val _results = MutableLiveData<List<String>>()
    val results: LiveData<List<String>> get() = _results

    val index = MutableLiveData<MissingLettersIndex?>(null)

    val query = MutableLiveData("")

    init {
        index.observeForever { tryLookupIfStateValid() }
        query.observeForever {
            Timber.d("Query changed to \"%s\"", it)
            savedState.set("query", it)
            tryLookupIfStateValid()
        }

        savedState.get<String>("query")?.let { query.postValue(it) }
    }

    @AnyThread
    private fun tryLookupIfStateValid() {
        val index = this.index.value
        val query = this.query.value
        if (index == null) {
            Timber.d("State is invalid: index is null")
        } else if (query == null) {
            Timber.d("State is invalid: query is null")
        } else if (!index.ready) {
            Timber.d("State is invalid: index is not ready")
        } else {
            Timber.d("State is valid; dispatching lookup")
            searchAndUpdateResults(index, query, MAX_RESULTS)
        }
    }

    @AnyThread
    private fun searchAndUpdateResults(index: MissingLettersIndex, query: String, maxResults: Int) {
        val input = Normalizer.normalize(query, Normalizer.Form.NFKC)
        viewModelScope.launch(Dispatchers.IO) {
            val queryResults = index.findPartial(input, null, maxResults)
            Timber.i("Found %d results for pattern \"%s\"", queryResults.size, input)
            Arrays.sort(queryResults, index.collator)
            _results.postValue(queryResults.asList())
        }
    }

    companion object {
        const val MAX_RESULTS: Int = 500
    }
}

