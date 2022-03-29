package xyz.lukasz.xword

import android.app.Application
import android.content.Context
import android.view.inputmethod.InputMethodManager
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.HiltAndroidApp
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.hilt.components.SingletonComponent
import timber.log.Timber

@HiltAndroidApp
class CrosswordHelperApplication : Application() {

    override fun onCreate() {
        super.onCreate()
        Timber.plant(Timber.DebugTree())
    }

    @Module
    @InstallIn(SingletonComponent::class)
    object DependencyModule {

        @Provides
        fun provideInputMethodManager(@ApplicationContext context: Context): InputMethodManager {
            return context.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        }
    }

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }
}
