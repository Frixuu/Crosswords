#ifndef CROSSWORD_HELPER_ANDROID_HPP
#define CROSSWORD_HELPER_ANDROID_HPP

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <jni.h>
#include <string>

namespace crossword::utils::android {

    /// Copies contents of a Java string into a new std::string.
    std::string string_from_java(JNIEnv *env, jstring jstr) {
        if (jstr != nullptr) {
            auto char_ptr = env->GetStringUTFChars(jstr, 0);
            auto str = std::string(char_ptr);
            env->ReleaseStringUTFChars(jstr, char_ptr);
            return str;
        } else {
            return std::string();
        }
    }

    /// Provides access to a read-only Android asset.
    class Asset final {
    private:
        AAsset* asset;
    public:
        Asset(AAsset* asset) : asset(asset) {}
        Asset(const Asset&) = delete;
        Asset(Asset&&) = delete;
        Asset& operator=(const Asset&) = delete;
        Asset& operator=(Asset&&) = delete;
        ~Asset() {
            if (asset != nullptr) {
                AAsset_close(asset);
            }
        }

        /// Gets a pointer to a buffer holding the contents of the asset.
        /// Returns nullptr on failure.
        inline const char* get_buffer() {
            return static_cast<const char*>(AAsset_getBuffer(asset));
        }

        /// Reports the total size of the asset.
        inline off_t length() const {
            return AAsset_getLength(asset);
        }

        inline int open_file_descriptor(off_t* out_start, off_t* out_length) const {
            return AAsset_openFileDescriptor(asset, out_start, out_length);
        }
    };

    enum AssetOpenMode : int {
        /** No specific information about how data will be accessed. **/
        Unknown = 0,
        /** Read chunks, and seek forward and backward. */
        Random = 1,
        /** Read sequentially, with an occasional forward seek. */
        Streaming = 2,
        /** Caller plans to ask for a read-only buffer with all data. */
        Buffer = 3
    };

    class AssetManager {
    private:

        AAssetManager* mgr;

        AssetManager(JNIEnv *env, jobject assetManager)
            : mgr(AAssetManager_fromJava(env, assetManager)) {
        }

        AssetManager(const AssetManager&) = delete;
        AssetManager& operator=(const AssetManager&) = delete;
        AssetManager(AssetManager&&) = delete;
        AssetManager& operator=(AssetManager&&) = delete;

    public:

        inline Asset open_asset(std::string& path, AssetOpenMode open_mode) {
            return Asset(AAssetManager_open(mgr, path.c_str(), open_mode));
        }

        inline static AssetManager from_java(JNIEnv *env, jobject assetManager) {
            return AssetManager(env, assetManager);
        }
    };

    template<typename ...Args>
    [[maybe_unused]] inline void logf(android_LogPriority priority,
                                      const char *tag,
                                      const char *fmt,
                                      Args... args) {

        __android_log_print(priority, tag, fmt, args...);
    }

    template<typename ...Args>
    [[maybe_unused]] inline void infof(const char *tag,
                                       const char *fmt,
                                       Args... args) {

        logf(ANDROID_LOG_INFO, tag, fmt, args...);
    }

    template<typename ...Args>
    [[maybe_unused]] inline void infof(const char *fmt,
                                       Args... args) {

        infof("Native", fmt, args...);
    }
}

#endif //CROSSWORD_HELPER_ANDROID_HPP
