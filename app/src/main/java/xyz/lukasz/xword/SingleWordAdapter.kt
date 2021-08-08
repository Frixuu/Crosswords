package xyz.lukasz.xword

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import xyz.lukasz.xword.databinding.ItemSingleWordBinding

class SingleWordAdapter(
    private val data: List<String>,
    private val activity: MainActivity
    ) : RecyclerView.Adapter<SingleWordAdapter.ViewHolder>() {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val inflater = LayoutInflater.from(parent.context)
        val binding = ItemSingleWordBinding.inflate(inflater, parent, false)
        return ViewHolder(binding, activity)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        holder.bind(data[position])
    }

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