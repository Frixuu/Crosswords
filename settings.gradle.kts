rootProject.name = "Crossword Helper"
include(":app")

dependencyResolutionManagement {
    versionCatalogs {
        create("libs") {
            version("hilt", "2.41")
            version("kotlin", "1.6.10")
        }
    }
}
