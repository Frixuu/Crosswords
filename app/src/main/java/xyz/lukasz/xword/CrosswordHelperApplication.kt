package xyz.lukasz.xword

import android.app.Application
import dagger.hilt.android.HiltAndroidApp

@HiltAndroidApp
class CrosswordHelperApplication : Application() {

    override fun onCreate() {
        super.onCreate()
    }
}
