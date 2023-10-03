import com.nishtahir.CargoBuildTask

plugins {
    id("com.android.application")
    kotlin("android")
    kotlin("kapt")
    id("dagger.hilt.android.plugin")
    id("org.mozilla.rust-android-gradle.rust-android")
}

android {

    namespace = "xyz.lukasz.xword"
    compileSdk = 34
    buildToolsVersion = "34.0.0"

    androidResources {
        // Compressed resources cannot be mmapped.
        // We leave dictionary files uncompressed to speed up their loading
        noCompress.add("txt")
    }

    buildFeatures {
        dataBinding = true
        viewBinding = true
    }

    defaultConfig {
        applicationId = "xyz.lukasz.xword"
        minSdk = 23
        targetSdk = 34
        versionCode = 3
        versionName = "1.2"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        externalNativeBuild {
            cmake {
                cppFlags("-std=c++20 -Wall -Wextra -pedantic -frtti -fno-exceptions")
            }
        }

        ndk {
            abiFilters.addAll(setOf("x86_64", "armeabi-v7a", "arm64-v8a"))
        }
    }

    buildTypes {
        getByName("release") {
            isShrinkResources = true
            isMinifyEnabled = true
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }

    ndkVersion = "26.0.10792818"
    externalNativeBuild {
        cmake {
            path("CMakeLists.txt")
        }
    }

    kotlinOptions {
        jvmTarget = "1.8"
    }
}

cargo {
    module = "src/main/rust"
    libname = "crossword"
    targets = listOf("arm", "arm64", "x86_64")
}

dependencies {

    implementation(libs.kotlin.stdlib)
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:1.6.4")

    implementation(libs.hilt.android)
    kapt(libs.hilt.compiler)

    implementation("androidx.core:core-ktx:1.12.0")
    implementation("androidx.appcompat:appcompat:1.6.1")
    implementation("com.android.volley:volley:1.2.1")
    implementation("com.google.android.material:material:1.9.0")
    implementation("androidx.constraintlayout:constraintlayout:2.1.4")
    implementation("androidx.fragment:fragment-ktx:1.6.1")
    implementation("androidx.slidingpanelayout:slidingpanelayout:1.2.0")
    implementation("net.danlew:android.joda:2.10.14")
    implementation("androidx.legacy:legacy-support-v4:1.0.0")
    implementation("org.jsoup:jsoup:1.14.3")
    implementation("com.squareup.okhttp3:okhttp:4.9.3")
    implementation("com.jakewharton.timber:timber:5.0.1")
    implementation("androidx.lifecycle:lifecycle-viewmodel-ktx:${libs.versions.lifecycleViewmodel.get()}")
    implementation("androidx.navigation:navigation-fragment-ktx:${libs.versions.navigation.get()}")
    implementation("androidx.navigation:navigation-ui-ktx:${libs.versions.navigation.get()}")

    testImplementation("junit:junit:4.13.2")
    androidTestImplementation("androidx.test.ext:junit:1.1.5")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.5.1")
}

kapt {
    correctErrorTypes = true
}

tasks.preBuild.configure {
    dependsOn.add(tasks.withType(CargoBuildTask::class.java))
}
