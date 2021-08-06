package xyz.lukasz.xword

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import xyz.lukasz.xword.databinding.SingleWordLayoutBinding

class SingleWordAdapter(
    private val data: List<String>,
    ) : RecyclerView.Adapter<SingleWordAdapter.ViewHolder>() {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val inflater = LayoutInflater.from(parent.context)
        val binding = SingleWordLayoutBinding.inflate(inflater, parent, false)
        return ViewHolder(binding)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        holder.bind(data[position])
    }

    override fun getItemCount(): Int {
        return data.size
    }

    inner class ViewHolder(
        private val binding: SingleWordLayoutBinding)
        : RecyclerView.ViewHolder(binding.root) {

        fun bind(model: String) {
            binding.word = model
        }
    }
}