package xyz.lukasz.xword

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import xyz.lukasz.xword.databinding.ItemSingleWordBinding
import xyz.lukasz.xword.utils.ObjectEqualsCallback

/**
 * Binds word list to a RecyclerView.
 */
class SingleWordAdapter(
    private val activity: MainActivity
) : ListAdapter<String, SingleWordAdapter.ViewHolder>(ObjectEqualsCallback()) {

    /**
     * Creates a new ViewHolder for a single word.
     */
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val inflater = LayoutInflater.from(parent.context)
        val binding = ItemSingleWordBinding.inflate(inflater, parent, false)
        return ViewHolder(binding, activity)
    }

    /**
     * Binds a single item in the list to a ViewHolder.
     */
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        holder.bind(getItem(position))
    }

    inner class ViewHolder(
        private val binding: ItemSingleWordBinding,
        activity: MainActivity
    ) : RecyclerView.ViewHolder(binding.root) {

        init {
            binding.activity = activity
        }

        fun bind(model: String) {
            binding.word = model
        }
    }
}
