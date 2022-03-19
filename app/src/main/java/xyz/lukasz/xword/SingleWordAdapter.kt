package xyz.lukasz.xword

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import xyz.lukasz.xword.databinding.ItemSingleWordBinding

/**
 * Binds word list to a RecyclerView.
 */
class SingleWordAdapter(
    private val data: List<String>,
    private val activity: MainActivity
) : RecyclerView.Adapter<SingleWordAdapter.ViewHolder>() {

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
        holder.bind(data[position])
    }

    /**
     * Returns the number of words in this adapter.
     */
    override fun getItemCount(): Int {
        return data.size
    }

    inner class ViewHolder(
        private val binding: ItemSingleWordBinding,
        private val activity: MainActivity
    ) : RecyclerView.ViewHolder(binding.root) {

        fun bind(model: String) {
            binding.word = model
            binding.activity = activity
        }
    }
}
