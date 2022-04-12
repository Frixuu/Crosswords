package xyz.lukasz.xword.search

import android.content.Context
import android.content.res.AssetManager
import android.view.View
import androidx.annotation.AnyThread
import androidx.lifecycle.*
import androidx.transition.Fade
import androidx.transition.TransitionManager
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import timber.log.Timber
import xyz.lukasz.xword.R
import xyz.lukasz.xword.utils.showSnackbar
import javax.inject.Inject

@HiltViewModel
class SearchResultsViewModel @Inject constructor(
    private val savedState: SavedStateHandle
) : ViewModel() {

    private val _results = MutableLiveData<List<String>>()
    val results: LiveData<List<String>> get() = _results

    val index = MutableLiveData<WordIndex?>(null)
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
    fun switchIndexCategory(assetManager: AssetManager, mode: WordIndexType) {
        viewModelScope.launch(Dispatchers.Main) {
            index.value?.unload()
            index.value = null
            withContext(Dispatchers.IO) {
                val newIndex = WordIndexFactory.create(mode)
                newIndex.loadFromAsset(assetManager)
                index.postValue(newIndex)
            }
        }
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
    private fun searchAndUpdateResults(index: WordIndex, query: String, maxResults: Int) {
        viewModelScope.launch(Dispatchers.IO) {
            val queryResults = index.lookup(query, maxResults)
            Timber.i("Found %d results for pattern \"%s\"", queryResults.size, query)
            queryResults.sortWith(index.collator)
            _results.postValue(queryResults)
        }
    }

    override fun onCleared() {
        super.onCleared()
    }

    companion object {
        const val MAX_RESULTS: Int = 500
    }
}

