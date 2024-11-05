#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <iomanip>
#include <filesystem>
#include <cctype>

namespace fs = std::filesystem;
using namespace std;

set<string> stopWords = {"A", "AND", "AN", "OF", "IN", "THE"};

map<string, double> processFile(const string &filename);
double calculateSimilarity(const map<string, double> &freq1, const map<string, double> &freq2);

int main() {
    string directoryPath = R"(C:\Users\vikas\Downloads\Book-Txt)";
    vector<string> filenames;
    vector<map<string, double>> wordFrequencies;
    vector<vector<double>> similarityMatrix;

    // Step 1: Read all .txt files from the directory
    for (const auto &entry : fs::directory_iterator(directoryPath)) {
        if (entry.path().extension() == ".txt") {
            filenames.push_back(entry.path().string());
        }
    }

    int numFiles = filenames.size();
    similarityMatrix.resize(numFiles, vector<double>(numFiles, 0.0));

    // Step 2: Process each file to get normalized word frequencies
    for (int i = 0; i < numFiles; i++) {
        wordFrequencies.push_back(processFile(filenames[i]));
    }

    // Step 3: Calculate similarity matrix
    for (int i = 0; i < numFiles; i++) {
        for (int j = i + 1; j < numFiles; j++) {
            similarityMatrix[i][j] = calculateSimilarity(wordFrequencies[i], wordFrequencies[j]);
            similarityMatrix[j][i] = similarityMatrix[i][j];
        }
    }

    // Step 4: Find top 10 most similar pairs
    vector<tuple<double, int, int>> similarities;
    for (int i = 0; i < numFiles; i++) {
        for (int j = i + 1; j < numFiles; j++) {
            similarities.push_back(make_tuple(similarityMatrix[i][j], i, j));
        }
    }

    sort(similarities.begin(), similarities.end(), greater<tuple<double, int, int>>());

    // Step 5: Output top 10 similar pairs
    cout << "Top 10 similar pairs:\n";
    for (int k = 0; k < 10 && k < similarities.size(); k++) {
        double sim;
        int file1, file2;
        tie(sim, file1, file2) = similarities[k];
        cout << filenames[file1] << " and " << filenames[file2] << " - Similarity: " << fixed << setprecision(4) << sim << endl;
    }

    return 0;
}

// Function to process a file and return normalized word frequencies
map<string, double> processFile(const string &filename) {
    ifstream file(filename);
    map<string, int> wordCount;
    int totalWords = 0;
    
    if (file.is_open()) {
        string word;
        while (file >> word) {
            // Convert word to uppercase and remove non-alphabetic characters
            transform(word.begin(), word.end(), word.begin(), ::toupper);
            word.erase(remove_if(word.begin(), word.end(), [](char c) { return !isalnum(c); }), word.end());

            // Skip stop words
            if (stopWords.count(word) == 0 && !word.empty()) {
                wordCount[word]++;
                totalWords++;
            }
        }
        file.close();
    }

    // Keep only top 100 words by frequency
    vector<pair<string, int>> freqVec(wordCount.begin(), wordCount.end());
    sort(freqVec.begin(), freqVec.end(), [](const auto &a, const auto &b) { return a.second > b.second; });
    if (freqVec.size() > 100) freqVec.resize(100);

    // Normalize frequencies
    map<string, double> normalizedFreq;
    for (const auto &pair : freqVec) {
        normalizedFreq[pair.first] = static_cast<double>(pair.second) / totalWords;
    }

    return normalizedFreq;
}

// Function to calculate similarity between two files' word frequencies
double calculateSimilarity(const map<string, double> &freq1, const map<string, double> &freq2) {
    double similarity = 0.0;
    for (const auto &entry : freq1) {
        if (freq2.count(entry.first) > 0) {
            similarity += entry.second + freq2.at(entry.first);
        }
    }
    return similarity;
}