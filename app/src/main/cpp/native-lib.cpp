#include "native-lib.hpp"
#include "string_utils.hpp"
#include <android/asset_manager.h>
#include <jni.h>
#include <string>

using namespace crossword;

extern "C" JNIEXPORT jint JNICALL
Java_xyz_lukasz_xword_MainActivity_fooFromNative(JNIEnv *env,
                                                 [[maybe_unused]] jobject thiz) {
    return 4;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_xyz_lukasz_xword_Dictionary_loadNative(JNIEnv *env,
                                            [[maybe_unused]] jobject thiz,
                                            jobject assetMgr,
                                            jstring path,
                                            jlong prev_ptr,
                                            jint concLevel) {

    // If the caller specified an address of a previously created Dictionary, delete it
    // TODO: consider using a custom arena?
    auto dictionary = reinterpret_cast<Dictionary *>(prev_ptr);
    delete dictionary;

    // Assign null so that we can return a valid value even if asset mmapping fails
    dictionary = nullptr;

    // Mmap the whole uncompressed file
    auto assetManager = AAssetManager_fromJava(env, assetMgr);
    auto filename = env->GetStringUTFChars(path, nullptr);
    auto asset = AAssetManager_open(assetManager, filename, AASSET_MODE_BUFFER);

    off_t start = 0;
    off_t length = AAsset_getLength(asset);

    auto fd = AAsset_openFileDescriptor(asset, &start, &length);
    if (fd >= 0) {
        auto buffer = AAsset_getBuffer(asset);
        if (buffer != nullptr) {
            dictionary = new Dictionary();
            dictionary->load_from_buffer_par(static_cast<const char *>(buffer), length, concLevel);
        }
    }

    AAsset_close(asset);
    env->ReleaseStringUTFChars(path, filename);
    return reinterpret_cast<jlong>(dictionary);
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_xyz_lukasz_xword_Dictionary_findPartialNative(JNIEnv *env,
                                                   [[maybe_unused]] jobject thiz,
                                                   jlong native_ptr,
                                                   jstring jword,
                                                   jstring jcursor,
                                                   jint maxCount) {

    // Marshal Java arguments to native
    auto word = crossword::utils::string_from_java(env, jword);
    auto cursor = crossword::utils::string_from_java(env, jcursor);
    auto dictionary = reinterpret_cast<Dictionary *>(native_ptr);

    // Find all the matching words
    std::vector<std::string> resultVec;
    dictionary->find_words(resultVec, word);

    // Map found words to a Java string array
    auto stringClazz = env->FindClass("java/lang/String");
    auto results = env->NewObjectArray(resultVec.size(), stringClazz, nullptr);
    for (int i = 0; i < resultVec.size(); ++i) {
        env->SetObjectArrayElement(results, i, env->NewStringUTF(resultVec.at(i).c_str()));
    }
    return results;
}