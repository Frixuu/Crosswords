package xyz.lukasz.xword

import android.os.Bundle
import android.view.inputmethod.InputMethodManager
import androidx.annotation.LayoutRes
import androidx.appcompat.app.AppCompatActivity
import androidx.databinding.DataBindingUtil
import androidx.databinding.ViewDataBinding
import javax.inject.Inject

/**
 * Base class for all activities in the application.
 */
abstract class ActivityBase<TBinding : ViewDataBinding>(@LayoutRes val layout: Int)
    : AppCompatActivity() {

    @Inject lateinit var inputMethodManager: InputMethodManager

    /**
     * Generated data binding object.
     */
    open lateinit var binding: TBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = DataBindingUtil.setContentView(this, layout)
    }

    /**
     * If some focused element is accepting text,
     * blurs it and hides the software keyboard.
     */
    protected fun hideSoftwareKeyboard() {
        if (inputMethodManager.isAcceptingText) {
            val focus = currentFocus ?: return
            inputMethodManager.hideSoftInputFromWindow(focus.windowToken, 0)
            focus.clearFocus()
        }
    }
}
