buildscript {
    dependencies {
        classpath("com.android.tools.build:gradle:8.1.2")
        classpath("org.jetbrains.kotlin:kotlin-gradle-plugin:${libs.versions.kotlin.get()}")
    }
}

allprojects {
    repositories {
        google()
        mavenCentral()
    }
}

plugins {
    id("org.mozilla.rust-android-gradle.rust-android").version("0.9.3")
    id("com.google.dagger.hilt.android").version(libs.versions.hilt.get()).apply(false)
}


tasks.register<Delete>("clean") {
    delete(rootProject.buildDir)
}
