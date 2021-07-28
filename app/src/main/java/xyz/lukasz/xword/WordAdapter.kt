package xyz.lukasz.xword

import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView

class WordViewHolder(view: View) : RecyclerView.ViewHolder(view) {
    val wordView: TextView = view.findViewById(R.id.article_title)
}

class WordAdapter(
    val data: List<String>,
    val recyclerView: RecyclerView
    ) : RecyclerView.Adapter<WordViewHolder>() {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): WordViewHolder {
        val view = LayoutInflater
            .from(parent.context)
            .inflate(R.layout.single_word_layout, parent, false)
        view.setOnClickListener(View.OnClickListener {
            Log.i("WAdapter", it.toString())
        })
        return WordViewHolder(view)
    }

    override fun onBindViewHolder(holder: WordViewHolder, position: Int) {
        val word = data[position]
        holder.wordView.text = word
    }

    override fun getItemCount(): Int {
        return data.size
    }

}