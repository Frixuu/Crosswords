#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <jni.h>
#include <string>
#include "dictionary.hpp"
#include "utils/android.hpp"
#include "utils/utf8.hpp"

using namespace crossword::utils;
using crossword::Dictionary;

extern "C"
JNIEXPORT jlong JNICALL
Java_xyz_lukasz_xword_Dictionary_loadNative(JNIEnv *env,
                                            [[maybe_unused]] jobject thiz,
                                            jobject jasset_mgr,
                                            jstring path,
                                            jlong prev_ptr,
                                            jint par_level) {

    // If the caller specified an address of a previously created Dictionary, delete it
    auto dictionary = reinterpret_cast<Dictionary *>(prev_ptr);
    delete dictionary;

    // Assign null so that we can return a valid value even if asset mmapping fails
    dictionary = nullptr;

    // Mmap the whole uncompressed file
    auto asset_manager = AAssetManager_fromJava(env, jasset_mgr);
    auto filename = env->GetStringUTFChars(path, nullptr);
    auto asset = AAssetManager_open(asset_manager, filename, AASSET_MODE_BUFFER);

    off_t start = 0;
    off_t length = AAsset_getLength(asset);

    auto fd = AAsset_openFileDescriptor(asset, &start, &length);
    if (fd >= 0) {
        auto buffer = AAsset_getBuffer(asset);
        if (buffer != nullptr) {
            dictionary = new Dictionary();
            dictionary->load_from_buffer_par(static_cast<const char *>(buffer), length, par_level);
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
                                                   jint limit) {

    // Marshal Java arguments to native
    auto word = android::string_from_java(env, jword);
    auto cursor = android::string_from_java(env, jcursor);
    auto dictionary = reinterpret_cast<Dictionary *>(native_ptr);

    // Find all the matching words
    std::vector<std::string> result_vec;
    dictionary->find_words(result_vec, word, limit, cursor);

    // Map found words to a Java string array
    auto string_clazz = env->FindClass("java/lang/String");
    auto results = env->NewObjectArray(result_vec.size(), string_clazz, nullptr);
    for (size_t i = 0; i < result_vec.size(); ++i) {
        env->SetObjectArrayElement(results, i, env->NewStringUTF(result_vec.at(i).c_str()));
    }

    return results;
}