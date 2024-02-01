buildscript {
    dependencies {
        classpath("com.android.tools.build:gradle:8.2.1")
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
    id("com.google.dagger.hilt.android").version(libs.versions.hilt.get()).apply(false)
}

tasks.register<Delete>("clean") {
    delete(rootProject.buildDir)
}
