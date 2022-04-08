#include "indexing/missing_letters.hpp"
#include "indexing/word_index.hpp"
#include "interop/pointer_wrapper.hpp"
#include "interop/strings.hpp"
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
using crossword::indexing::WordIndex;
using crossword::utils::android::AssetManager;
using crossword::utils::android::AssetOpenMode;

extern "C" JNIEXPORT jobject JNICALL
Java_xyz_lukasz_xword_search_MissingLettersIndex_loadNative(JNIEnv* env,
                                                            [[maybe_unused]] jobject thiz,
                                                            jobject jasset_mgr,
                                                            jstring path,
                                                            jint thread_count) {
    // Mmap the whole uncompressed file
    auto filename = interop::copy_utf8_string(env, path);
    auto asset_manager = AssetManager::from_java(env, jasset_mgr);
    auto asset = asset_manager.open_asset(filename, AssetOpenMode::Buffer);

    off_t start = 0;
    off_t length = asset.length();

    auto index = std::make_shared<MissingLettersIndex>();
    auto fd = asset.open_file_descriptor(&start, &length);
    if (fd >= 0) {
        auto buffer = asset.get_buffer();
        if (buffer != nullptr) {
            index->load_from_buffer_par(buffer, static_cast<int>(length), thread_count);
        }
    }

    return interop::wrap_shared_ptr(env, std::move(index));
}

extern "C" JNIEXPORT jobjectArray JNICALL
Java_xyz_lukasz_xword_search_WordIndex_lookupNative(JNIEnv* env,
                                                    [[maybe_unused]] jobject thiz,
                                                    jlong native_ptr,
                                                    jstring jquery,
                                                    jint maxResults) {
    // Marshal Java arguments to native
    auto query = interop::copy_utf8_string(env, jquery);
    auto index = interop::unwrap_shared_ptr<WordIndex>(native_ptr);

    // Find all the matching words
    auto result_vec = index->lookup(query, maxResults);

    // Map found words to a Java string array
    auto string_clazz = env->FindClass("java/lang/String");
    auto array_size = static_cast<jsize>(result_vec.size());
    auto results = env->NewObjectArray(array_size, string_clazz, nullptr);
    for (size_t i = 0; i < result_vec.size(); ++i) {
        auto c_str = reinterpret_cast<const char*>(result_vec.at(i).c_str());
        auto found_word = env->NewStringUTF(c_str);
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
