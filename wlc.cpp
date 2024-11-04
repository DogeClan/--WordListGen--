#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <fstream>
#include <mutex>
#include <queue>

class WordListGenerator {
public:
    WordListGenerator(const std::string& characters, int minLength, int maxLength)
        : characters(characters), minLength(minLength), maxLength(maxLength) {}

    void generateWords() {
        std::vector<std::thread> threads;
        for (int length = minLength; length <= maxLength; ++length) {
            std::string word(length, ' ');
            threads.emplace_back([this, word, length]() mutable {
                generateCombinations(word, length, 0);
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }

    void saveToFile(const std::string& filename) {
        std::ofstream outFile(filename, std::ios::out | std::ios::app);
        if (!outFile.is_open()) {
            std::cerr << "Error opening file for writing." << std::endl;
            return;
        }

        std::string word;
        while (true) {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                if (generatedWordsQueue.empty()) {
                    break;
                }
                word = std::move(generatedWordsQueue.front());
                generatedWordsQueue.pop();
            }
            outFile << word << std::endl;
        }
        outFile.close();
        std::cout << "Word list saved to " << filename << std::endl;
    }

private:
    std::string characters;
    int minLength;
    int maxLength;
    std::queue<std::string> generatedWordsQueue;
    std::mutex queueMutex;

    void generateCombinations(std::string& word, int length, int position) {
        if (position == length) {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                generatedWordsQueue.push(word);
            }
            return;
        }

        for (char c : characters) {
            word[position] = c;
            generateCombinations(word, length, position + 1);
        }
    }
};

int main() {
    const std::string characters =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "!@#$%^&*()-_=+[]{};:,.<>?";

    int minLength = 1;
    int maxLength = 6; 

    WordListGenerator generator(characters, minLength, maxLength);

    // Change the filename here to "wl.txt"
    std::thread writingThread(&WordListGenerator::saveToFile, &generator, "wl.txt");

    generator.generateWords();

    writingThread.join();
    return 0;
}
