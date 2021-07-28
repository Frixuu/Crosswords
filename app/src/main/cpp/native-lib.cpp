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
JNIEXPORT jobjectArray JNICALL
Java_xyz_lukasz_xword_dictionaries_Dictionary_findPartialNative(JNIEnv *env,
                                                                jobject thiz,
                                                                jlong native_ptr,
                                                                jstring jword) {

    jobjectArray results;
    auto word = env->GetStringUTFChars(jword, 0);
    Dictionary* dictionary = reinterpret_cast<Dictionary*>(native_ptr);
    std::vector<std::string> resultVec;
    dictionary->find_word(resultVec, word);
    results = (jobjectArray)env->NewObjectArray(
            resultVec.size(),
            env->FindClass("java/lang/String"),
            env->NewStringUTF(""));
    for (int i = 0; i < resultVec.size(); i++) {
        env->SetObjectArrayElement(results, i, env->NewStringUTF(resultVec.at(i).c_str()));
    }
    env->ReleaseStringUTFChars(jword, word);
    return results;
}