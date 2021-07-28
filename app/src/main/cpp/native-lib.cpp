#include "native-lib.h"
#include <android/asset_manager.h>
#include <jni.h>
#include <string>

extern "C" JNIEXPORT jint JNICALL Java_xyz_lukasz_xword_MainActivity_fooFromNative(JNIEnv *env, jobject thiz) {
    return 4;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_xyz_lukasz_xword_dictionaries_Dictionary_loadNative(JNIEnv *env,
                                                         jobject thiz,
                                                         jobject assetMgr,
                                                         jstring path,
                                                         jlong oldPtr,
                                                         jint concLevel) {

    if (oldPtr != 0) {
        delete reinterpret_cast<Dictionary*>(oldPtr);
    }

    Dictionary* dictionary = nullptr;

    auto assetManager = AAssetManager_fromJava(env, assetMgr);
    auto filename = env->GetStringUTFChars(path, 0);
    auto asset = AAssetManager_open(assetManager, filename, AASSET_MODE_BUFFER);

    off_t start = 0;
    off_t length = AAsset_getLength(asset);

    auto fd = AAsset_openFileDescriptor(asset, &start, &length);
    if (fd >= 0) {
        auto buffer = AAsset_getBuffer(asset);
        if (buffer != nullptr) {
            dictionary = new Dictionary();
            dictionary->load_from_android_asset(static_cast<const char*>(buffer), length, concLevel);
        }
    }

    AAsset_close(asset);
    env->ReleaseStringUTFChars(path, filename);
    return reinterpret_cast<jlong>(dictionary);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_xyz_lukasz_xword_dictionaries_Dictionary_findNative(JNIEnv *env,
                                                         jobject thiz,
                                                         jlong native_ptr,
                                                         jstring word) {
    return false;
}