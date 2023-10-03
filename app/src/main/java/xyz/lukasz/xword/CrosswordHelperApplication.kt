package xyz.lukasz.xword

import android.app.Application
import dagger.hilt.android.HiltAndroidApp
import timber.log.Timber

@HiltAndroidApp
class CrosswordHelperApplication : Application() {

    override fun onCreate() {
        super.onCreate()
        Timber.plant(Timber.DebugTree())
    }

    companion object {
        init {
            System.loadLibrary("native-lib")
            System.loadLibrary("crossword")
        }
    }
}
