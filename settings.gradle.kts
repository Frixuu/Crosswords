rootProject.name = "Crossword Helper"
include(":app")

dependencyResolutionManagement {
    versionCatalogs {
        create("libs") {
            version("hilt", "2.41")
            version("kotlin", "1.6.10")
            version("navigation", "2.4.1")
        }
    }
}
