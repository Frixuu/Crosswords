#include "native-lib.h"
#include <jni.h>
#include <string>

extern "C" JNIEXPORT jint JNICALL Java_xyz_lukasz_xword_MainActivity_fooFromNative(JNIEnv *env, jobject thiz) {
    return 4;
}
