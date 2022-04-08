#include "indexing/missing_letters.hpp"
#include "utils/android.hpp"
#include "utils/utf8.hpp"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <jni.h>
#include <memory>
#include <string>

using namespace crossword::utils;
using namespace crossword::utils::android;
using crossword::indexing::MissingLettersIndex;
using crossword::utils::android::AssetManager;
using crossword::utils::android::AssetOpenMode;

extern "C" JNIEXPORT jobject JNICALL
Java_xyz_lukasz_xword_search_MissingLettersIndex_loadNative(JNIEnv* env,
                                                            [[maybe_unused]] jobject thiz,
                                                            jobject jasset_mgr,
                                                            jstring path,
                                                            jint thread_count) {
    // Mmap the whole uncompressed file
    auto filename = android::string_from_java(env, path);
    auto asset_manager = AssetManager::from_java(env, jasset_mgr);
    auto asset = asset_manager.open_asset(filename, AssetOpenMode::Buffer);

    off_t start = 0;
    off_t length = asset.length();

    std::shared_ptr<MissingLettersIndex>* index = nullptr;
    auto fd = asset.open_file_descriptor(&start, &length);
    if (fd >= 0) {
        auto buffer = asset.get_buffer();
        if (buffer != nullptr) {
            index = new std::shared_ptr<MissingLettersIndex>(new MissingLettersIndex());
            index->get()->load_from_buffer_par(buffer, static_cast<int>(length), thread_count);
        }
    }

    auto wrapper_class = env->FindClass("xyz/lukasz/xword/interop/NativeSharedPointer");
    auto ctor = env->GetMethodID(wrapper_class, "<init>", "(J)V");
    auto wrapper = env->NewObject(wrapper_class, ctor, reinterpret_cast<jlong>(index));
    return wrapper;
}

extern "C" JNIEXPORT jobjectArray JNICALL
Java_xyz_lukasz_xword_search_MissingLettersIndex_findPartialNative(JNIEnv* env,
                                                                   [[maybe_unused]] jobject thiz,
                                                                   jlong native_ptr,
                                                                   jstring jword,
                                                                   jstring jcursor,
                                                                   jint limit) {
    // Marshal Java arguments to native
    auto word = android::string_from_java(env, jword);
    auto cursor = android::string_from_java(env, jcursor);
    auto index = *reinterpret_cast<std::shared_ptr<MissingLettersIndex>*>(native_ptr);

    // Find all the matching words
    auto result_vec = index->lookup(word, limit);

    // Map found words to a Java string array
    auto string_clazz = env->FindClass("java/lang/String");
    auto array_size = static_cast<jsize>(result_vec.size());
    auto results = env->NewObjectArray(array_size, string_clazz, nullptr);
    for (size_t i = 0; i < result_vec.size(); ++i) {
        auto found_word = env->NewStringUTF(result_vec.at(i).c_str());
        env->SetObjectArrayElement(results, static_cast<jsize>(i), found_word);
        env->DeleteLocalRef(found_word);
    }

    return results;
}

extern "C" JNIEXPORT void JNICALL
Java_xyz_lukasz_xword_interop_NativeSharedPointer_freeImpl([[maybe_unused]] JNIEnv* env,
                                                           [[maybe_unused]] jclass clazz,
                                                           jlong ptr) {
    auto ptr_to_shared = reinterpret_cast<std::shared_ptr<void*>*>(ptr);
    if (!ptr_to_shared->unique()) {
        auto count = ptr_to_shared->use_count();
        log::tag("freeImpl").i("Pointer not unique, it has %d more refs", count - 1);
    }

    delete ptr_to_shared;
}
