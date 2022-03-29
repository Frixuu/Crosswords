#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <jni.h>
#include <memory>
#include <string>
#include "dictionary.hpp"
#include "utils/android.hpp"
#include "utils/utf8.hpp"

using namespace crossword::utils;
using crossword::Dictionary;
using crossword::utils::android::AssetManager;
using crossword::utils::android::AssetOpenMode;

extern "C"
JNIEXPORT jobject JNICALL
Java_xyz_lukasz_xword_Dictionary_loadNative(JNIEnv *env,
                                            [[maybe_unused]] jobject thiz,
                                            jobject jasset_mgr,
                                            jstring path,
                                            jint par_level) {

    // Mmap the whole uncompressed file
    auto filename = android::string_from_java(env, path);
    auto asset_manager = AssetManager::from_java(env, jasset_mgr);
    auto asset = asset_manager.open_asset(filename, AssetOpenMode::Buffer);

    off_t start = 0;
    off_t length = asset.length();

    std::shared_ptr<Dictionary>* dictionary = nullptr;
    auto fd = asset.open_file_descriptor(&start, &length);
    if (fd >= 0) {
        auto buffer = asset.get_buffer();
        if (buffer != nullptr) {
            dictionary = new std::shared_ptr<Dictionary>(new Dictionary());
            dictionary->get()->load_from_buffer_par(buffer, static_cast<int>(length), par_level);
        }
    }

    auto wrapper_class = env->FindClass("xyz/lukasz/xword/interop/NativeSharedPointer");
    auto constructor_id = env->GetMethodID(wrapper_class, "<init>", "(J)V");
    auto wrapper = env->NewObject(wrapper_class, constructor_id, reinterpret_cast<jlong>(dictionary));
    return wrapper;
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
    auto dictionary = *reinterpret_cast<std::shared_ptr<Dictionary>*>(native_ptr);

    // Find all the matching words
    std::vector<std::string> result_vec;
    dictionary->find_words(result_vec, word, limit, cursor);

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

extern "C"
JNIEXPORT void JNICALL
Java_xyz_lukasz_xword_interop_NativeSharedPointer_freeImpl([[maybe_unused]] JNIEnv *env,
                                                           [[maybe_unused]] jclass clazz,
                                                           jlong ptr) {

    auto ptr_to_shared_pointer = reinterpret_cast<std::shared_ptr<void*>*>(ptr);
    delete ptr_to_shared_pointer;
}
