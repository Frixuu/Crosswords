rootProject.name = "Crossword Helper"
include(":app")

dependencyResolutionManagement {
    versionCatalogs {
        create("libs") {
            version("hilt", "2.48")
            version("kotlin", "1.9.0")
            version("navigation", "2.7.3")
            version("lifecycleViewmodel", "2.6.2")
        }
    }
}
