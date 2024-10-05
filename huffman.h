#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#define SHORT_MAX 32768 // B1000000000000000

struct CharacterFrequency {
    unsigned char character;
    unsigned int frequency;
};

struct HuffmanNode {
    CharacterFrequency characterFrequency{};
    std::string binaryCode;
    HuffmanNode* left = nullptr;
    HuffmanNode* right = nullptr;

    explicit HuffmanNode(const CharacterFrequency& characterFrequency) : characterFrequency(characterFrequency) {}
};

void compression(const std::string& fileName);

void decompression(const std::string& fileName);

std::vector<CharacterFrequency> extractCharactersFrequencies(const std::vector<std::string>& fileContents);

HuffmanNode* createHuffmanTree(const std::vector<CharacterFrequency>& charactersFrequencies);

std::unordered_map<unsigned char, std::string> remappingCharacters(HuffmanNode* root);

unsigned long long createTheCompressFile(const std::unordered_map<unsigned char, std::string>& charactersMapping,
                         const std::vector<CharacterFrequency>& charactersFrequencies,
                         const std::string& fileName,
                         const std::vector<std::string>& fileContents);

void createTheDecompressFile(const std::string& fileName);

void addDataInTheFile(const std::unordered_map<unsigned char, std::string>& charactersMapping, unsigned char character,
                      std::ofstream& file, unsigned short& step, unsigned short& number);
