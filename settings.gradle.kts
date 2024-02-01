rootProject.name = "Crossword Helper"
include(":app")

pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}

dependencyResolutionManagement {
    versionCatalogs {
        create("libs") {

            version("kotlin", "1.9.0")
            library("kotlin-stdlib", "org.jetbrains.kotlin", "kotlin-stdlib").versionRef("kotlin")

            version("hilt", "2.48")
            library("hilt-android", "com.google.dagger", "hilt-android").versionRef("hilt")
            library("hilt-compiler", "com.google.dagger", "hilt-compiler").versionRef("hilt")

            version("navigation", "2.7.6")
            library("navigation-fragment-ktx", "androidx.navigation", "navigation-fragment-ktx").versionRef("navigation")
            library("navigation-ui-ktx", "androidx.navigation", "navigation-ui-ktx").versionRef("navigation")

            version("lifecycleViewmodel", "2.6.2")
        }
    }
}
