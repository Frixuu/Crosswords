# Overrides
-optimizationpasses 10

# For debugging
-keepattributes SourceFile,LineNumberTable
-renamesourcefileattribute SourceFile

# Strip support libraries
-assumevalues class android.os.Build$VERSION {
    int SDK_INT = 21..2147483647;
}

-android

