#ifndef CROSSWORD_HELPER_NATIVE_LIB_H
#define CROSSWORD_HELPER_NATIVE_LIB_H

#include <android/asset_manager_jni.h>
#include <strings.h>
#include <map>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

struct WordNode {

    std::string validWord;
    std::unique_ptr<std::map<char, std::unique_ptr<WordNode>>> children;

    WordNode(const std::string&& word) {
        validWord = std::move(word);
    }

    WordNode(): WordNode(std::string()) {
    }

    bool isValid() {
        return !validWord.empty();
    }

    bool push_word(const std::string&& str, int32_t index) {
        if (!children) {
            children = std::make_unique<std::map<char, std::unique_ptr<WordNode>>>();
        }

        auto strLen = str.length();

        if (index < strLen) {
            auto entry = children->try_emplace(tolower(str.at(index)), std::make_unique<WordNode>());
            return entry.first->second->push_word(std::move(str), index + 1);
        } else if (index == (strLen - 1)) {
            auto entry = children->try_emplace(str.at(index), std::make_unique<WordNode>());
            entry.first->second->validWord = std::move(str);
        } else if (index == strLen) {
            validWord = std::move(str);
        } else {
            return false;
        }

        return true;
    }

    void find_word(std::vector<std::string>& vec, const std::string& pattern, int32_t index) {
        if (!children || index > pattern.length()) {
            return;
        } else if (index == pattern.length()) {
            if (!validWord.empty()) {
                auto wordCopy = validWord;
                vec.emplace_back(std::move(wordCopy));
            }
            return;
        } else {
            auto ch = pattern.at(index);
            if (ch == '.') {
                for (const auto& entry : *children) {
                    entry.second->find_word(vec, pattern, index + 1);
                }
            } else {
                auto result = children->find(ch);
                if (result != children->end()) {
                    result->second->find_word(vec, pattern, index + 1);
                }
            }
        }
    }
};

class Dictionary {
private:
    std::unique_ptr<WordNode> forwardIndex;

    static constexpr bool isCodepointSingleByte(const char b) {
        return (b & 0b10000000) != 0b10000000;
    }

    static constexpr bool isCodepointTwoBytes(const char b) {
        return (b & 0b11000000) == 0b11000000;
    }

    static constexpr bool isCodepointThreeBytes(const char b) {
        return (b & 0b11100000) == 0b11100000;
    }

    static constexpr bool isCodepointFourBytes(const char b) {
        return (b & 0b11110000) == 0b11110000;
    }

    static void load_fragment(const Dictionary* dict, const char* buffer, int start, int end) {
        std::vector<char> lineBuffer;
        auto index = start;
        auto bytesTillCodepointEnd = 0;
        while (index < end) {
            auto byte = buffer[index++];
            if (bytesTillCodepointEnd != 0) {
                bytesTillCodepointEnd--;
                lineBuffer.emplace_back(std::move(byte));
            } else {
                if (byte != '\n' && byte != '\r') {
                    lineBuffer.emplace_back(std::move(byte));
                    if (isCodepointSingleByte(byte)) {
                        continue;
                    } else if (isCodepointTwoBytes(byte)) {
                        bytesTillCodepointEnd = 1;
                    } else if (isCodepointThreeBytes(byte)) {
                        bytesTillCodepointEnd = 2;
                    } else if (isCodepointFourBytes(byte)) {
                        bytesTillCodepointEnd = 3;
                    }
                } else {
                    if (!lineBuffer.empty()) {
                        auto word = std::string(lineBuffer.begin(), lineBuffer.end());
                        lineBuffer.clear();
                        dict->forwardIndex->push_word(std::move(word), 0);
                    }
                }
            }
        }

        if (!lineBuffer.empty()) {
            auto word = std::string(lineBuffer.begin(), lineBuffer.end());
            dict->forwardIndex->push_word(std::move(word), 0);
        }
    }

public:
    Dictionary() {
        forwardIndex = std::make_unique<WordNode>();
    }

    void find_word(std::vector<std::string>& vec, const std::string& pattern) {
        forwardIndex->find_word(vec, pattern, 0);
    }

    void load_from_android_asset(const char *buffer, off_t length, int concLevel) {
        std::vector<std::thread> loadingThreads;

        auto indices = new int[concLevel];
        indices[0] = 0;

        for (auto i = 1; i < concLevel; i++) {
            int candidate = i * length / concLevel;
            while (candidate < length) {
                auto thisByte = buffer[candidate++];
                auto nextByte = buffer[candidate];
                // TODO: Handle UTF-8 codepoints
                if ((thisByte == '\n' || thisByte == '\r') && nextByte != '\n' && nextByte != '\r') {
                    indices[i] = candidate + 1;
                    break;
                }
            }
        }

        Dictionary* dict = this;

        for (auto i = 0; i < concLevel; i++) {
            auto startIndex = indices[i];
            int endIndex = length;
            if (i < (concLevel - 1)) {
                endIndex = indices[i + 1];
            }

            std::thread thread (load_fragment, dict, buffer, startIndex, endIndex);
            loadingThreads.emplace_back(std::move(thread));
        }

        for (auto& thread : loadingThreads) {
            thread.join();
        }
    }
};

#endif //CROSSWORD_HELPER_NATIVE_LIB_H
