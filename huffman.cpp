#include <unordered_map>
#include <vector>
#include <algorithm>
#include <queue>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "huffman.h"

void compression(const std::string& fileName) {
    if (!fileName.ends_with("txt")) {
        std::cout << "Incorrect file type. Only encode txt files\nEnd of encoding";
        exit(EXIT_FAILURE);
    }

    std::ifstream file(fileName);
    if (!file) {
        std::cerr << "Unable to open : " << fileName << std::endl;
        exit(EXIT_FAILURE);
    }

    printf("Uncompressed file size: %llu bytes\n", std::filesystem::file_size(fileName));
    printf("\nStart compressing...\n");

    std::vector<std::string> fileContents;
    std::string line;
    while (std::getline(file, line)) {
        fileContents.push_back(line);
    }

    const auto charactersFrequencies = extractCharactersFrequencies(fileContents);
    const auto root = createHuffmanTree(charactersFrequencies);
    const auto charactersMapping = remappingCharacters(root);

    const auto compressSize = createTheCompressFile(charactersMapping, charactersFrequencies,
                                                    fileName.substr(0, fileName.size() - 3).append("thc"),
                                                    fileContents);
    printf("End of encoding\n\n");

    printf("Compressed file size: %llu bytes\n", compressSize);
    printf("Compression ratio: %.2f%%\n",
           100 * static_cast<double>(compressSize) / static_cast<double>(std::filesystem::file_size(fileName)));
}

// Yeah, useless
void decompression(const std::string& fileName) {
    printf("\nStart decompressing...\n");
    createTheDecompressFile(fileName);
    printf("End of decoding\n");
}

std::vector<CharacterFrequency> extractCharactersFrequencies(const std::vector<std::string>& fileContents) {
    std::unordered_map<unsigned char, unsigned int> charactersFrequencies;
    std::vector<CharacterFrequency> result;

    bool moreThanOneLine = false;
    for (auto line : fileContents) {
        if (moreThanOneLine) line.insert(0, "\n");
        for (int i = 0; i < line.length(); i++) {
            auto it = charactersFrequencies.find(line[i]);

            if (it != charactersFrequencies.end()) charactersFrequencies[line[i]]++;
            else charactersFrequencies[line[i]] = 1;
        }
        moreThanOneLine = true;
    }

    for (auto [character, frequency] : charactersFrequencies) {
        result.push_back({character, frequency});
    }

    std::ranges::sort(result, [](const CharacterFrequency a, const CharacterFrequency b) {
        return a.frequency < b.frequency;
    });

    return result;
}

HuffmanNode* createHuffmanTree(const std::vector<CharacterFrequency>& charactersFrequencies) {
    printf("Creating the huffman tree...\n");
    std::vector<HuffmanNode*> nodes;
    for (const auto& characterFrequency : charactersFrequencies) {
        auto node = new HuffmanNode(characterFrequency);
        node->left = nullptr;
        node->right = nullptr;
        nodes.push_back(node);
    }

    HuffmanNode* node1 = nullptr;
    HuffmanNode* node2 = nullptr;
    HuffmanNode* node3 = nullptr;

    // Sort the vector before starting the creation of the tree
    std::ranges::sort(nodes, [](const HuffmanNode* a, const HuffmanNode* b) {
        return a->characterFrequency.frequency < b->characterFrequency.frequency;
    });

    while (nodes.size() > 1) {
        node1 = nodes[0];
        node2 = nodes[1];
        nodes.erase(nodes.begin(), nodes.begin() + 2);

        node3 = new HuffmanNode({
            .character = '#',
            .frequency = node1->characterFrequency.frequency + node2->characterFrequency.frequency
        });

        if (node1->characterFrequency.frequency <= node2->characterFrequency.frequency) {
            node3->left = node1;
            node3->right = node2;
        } else {
            node3->left = node2;
            node3->right = node1;
        }

        nodes.push_back(node3);

        std::ranges::sort(nodes, [](const HuffmanNode* a, const HuffmanNode* b) {
            return a->characterFrequency.frequency < b->characterFrequency.frequency;
        });
    }

    return nodes.front();
}

std::unordered_map<unsigned char, std::string> remappingCharacters(HuffmanNode* root) {
    printf("Remapping the characters...\n");
    std::unordered_map<unsigned char, std::string> mapping;
    std::queue<HuffmanNode*> nodesQueue;

    root->binaryCode = "";
    nodesQueue.push(root);

    while (!nodesQueue.empty()) {
        const auto temp = nodesQueue.front();
        if (temp) {
            if (temp->left)
                temp->left->binaryCode = temp->binaryCode + "0";

            if (temp->right)
                temp->right->binaryCode = temp->binaryCode + "1";

            if (!temp->left && !temp->right) mapping[temp->characterFrequency.character] = temp->binaryCode;

            nodesQueue.push(temp->left);
            nodesQueue.push(temp->right);
        }
        nodesQueue.pop();
        delete temp;
    }

    return mapping;
}

unsigned long long createTheCompressFile(const std::unordered_map<unsigned char, std::string>& charactersMapping,
                                         const std::vector<CharacterFrequency>& charactersFrequencies,
                                         const std::string& fileName,
                                         const std::vector<std::string>& fileContents) {
    std::ofstream file(fileName, std::ios::binary);

    // File identifier
    for (unsigned char identifier : {23, 10, 20, 04}) {
        file.write(reinterpret_cast<char*>(&identifier), sizeof(unsigned char));
    }

    // Writing the number of different characters to store
    unsigned short numberOfDifferentCharacters = charactersMapping.size();
    file.write(reinterpret_cast<char*>(&numberOfDifferentCharacters), sizeof(unsigned short));
    printf("Number of different characters: %d\n", numberOfDifferentCharacters);

    // Writing the number characters to store in all
    unsigned int totalNumberOfCharacters = 0;
    bool moreThanOneLine = false;
    for (const auto& line : fileContents) {
        if (moreThanOneLine) totalNumberOfCharacters++;
        totalNumberOfCharacters += line.size();
        moreThanOneLine = true;
    }
    file.write(reinterpret_cast<char*>(&totalNumberOfCharacters), sizeof(unsigned int));
    printf("Total number of characters: %d\n", totalNumberOfCharacters);

    // Writing the frequencies of each character
    for (auto [character, frequency] : charactersFrequencies) {
        file.write(reinterpret_cast<char*>(&character), sizeof(unsigned char));
        file.write(reinterpret_cast<char*>(&frequency), sizeof(unsigned int));
    }

    // Writing the characters in the file
    moreThanOneLine = false;
    unsigned short step = SHORT_MAX; // B1000000000000000
    unsigned short number = 0;
    for (const auto& line : fileContents) {
        if (moreThanOneLine)
            addDataInTheFile(charactersMapping, '\n', file, step, number);

        for (const unsigned char character : line)
            addDataInTheFile(charactersMapping, character, file, step, number);

        moreThanOneLine = true;
    }

    // Add the non-written bits to the file
    if (step != SHORT_MAX && step != 0) {
        file.write(reinterpret_cast<char*>(&number), sizeof(unsigned short));
    }

    file.close();
    return std::filesystem::file_size(fileName);
}

void createTheDecompressFile(const std::string& fileName) {
    if (!fileName.ends_with("thc")) {
        std::cerr << "Incorrect file type\nEnd of decoding";
        return;
    }

    std::ifstream fileToDecode(fileName, std::ios::binary);
    if (!fileToDecode) {
        std::cerr << "Unable to open : " << fileName << std::endl;
        std::cerr << "End of decoding";
        return;
    }

    // Verifying the identifier of the .thc
    unsigned char c;
    for (const unsigned char identifier : {23, 10, 20, 04}) {
        fileToDecode.read(reinterpret_cast<char*>(&c), sizeof(unsigned char));
        if (c != identifier) {
            fileToDecode.close();
            std::cerr << "Incorrect file's identifier\nEnd of decoding";
            return;
        }
    }

    // Reading the number of different characters to store
    unsigned short numberOfDifferentCharacters = 0;
    fileToDecode.read(reinterpret_cast<char*>(&numberOfDifferentCharacters), sizeof(unsigned short));
    printf("Number of different characters: %d\n", numberOfDifferentCharacters);

    // Reading the number characters to store in all
    unsigned int totalNumberOfCharacters = 0;
    fileToDecode.read(reinterpret_cast<char*>(&totalNumberOfCharacters), sizeof(unsigned int));
    printf("Total number of characters: %d\n", totalNumberOfCharacters);

    // Reading the frequencies of each character
    std::vector<CharacterFrequency> charactersFrequencies;
    for (int i = 0; i < numberOfDifferentCharacters; i++) {
        unsigned char character;
        unsigned int frequency;
        fileToDecode.read(reinterpret_cast<char*>(&character), sizeof(unsigned char));
        fileToDecode.read(reinterpret_cast<char*>(&frequency), sizeof(unsigned int));
        charactersFrequencies.push_back({.character = character, .frequency = frequency});
    }

    // Re-creating the huffman tree
    printf("Re-creating the huffman tree...\n");
    const auto HUFFMAN_ROOF_NODE = createHuffmanTree(charactersFrequencies);

    // Creating the decoded file
    std::ofstream finalFile(fileName + ".txt");

    unsigned short buffer;
    HuffmanNode* huffmanCursor = nullptr;
    std::string binariesStringBuffer;
    std::string charactersStringBuffer;
    unsigned short step;

    // Read the file in chunks
    printf("Reading the file in chunks...\n");
    huffmanCursor = HUFFMAN_ROOF_NODE;
    unsigned int index = 0;
    while (fileToDecode) {
        // Read the next chunk of data
        fileToDecode.read(reinterpret_cast<char*>(&buffer), sizeof(unsigned short));
        if (!fileToDecode) {
            break;
        }

        // Write the binary representation in the binaries string buffer
        step = SHORT_MAX;
        while (step) {
            binariesStringBuffer.append(step & buffer ? "1" : "0");
            step = step >> 1;
        }

        // Write the right character in the characters string buffer
        // We will read another chunk of data if we have less than 20 bits in the binaries string
        while (!binariesStringBuffer.empty() && totalNumberOfCharacters && index < binariesStringBuffer.size()) {
            while (huffmanCursor) {
                if (huffmanCursor->left == nullptr && huffmanCursor->right == nullptr) {
                    charactersStringBuffer.push_back(static_cast<char>(huffmanCursor->characterFrequency.character));
                    binariesStringBuffer.erase(0, index);
                    totalNumberOfCharacters--;

                    huffmanCursor = HUFFMAN_ROOF_NODE;
                    index = 0;
                    break;
                }

                if (index >= binariesStringBuffer.size()) break;

                if (binariesStringBuffer[index] == '0') huffmanCursor = huffmanCursor->left;
                if (binariesStringBuffer[index] == '1') huffmanCursor = huffmanCursor->right;

                index++;
            }
        }

        finalFile.write(charactersStringBuffer.data(), charactersStringBuffer.size());
        charactersStringBuffer.clear();
    }

    finalFile.close();
    fileToDecode.close();

    printf("Decompressed file size: %llu bytes\n", std::filesystem::file_size(fileName + ".txt"));

    // Deleting the huffman tree
    printf("Deleting the huffman tree...\n");
    std::queue<HuffmanNode*> nodesQueue;
    nodesQueue.push(HUFFMAN_ROOF_NODE);
    while (!nodesQueue.empty()) {
        const auto temp = nodesQueue.front();
        if (temp) {
            nodesQueue.push(temp->left);
            nodesQueue.push(temp->right);
        }
        nodesQueue.pop();
        delete temp;
    }
}

void addDataInTheFile(const std::unordered_map<unsigned char, std::string>& charactersMapping,
                      const unsigned char character,
                      std::ofstream& file, unsigned short& step, unsigned short& number) {
    for (const unsigned char bit : charactersMapping.at(character)) {
        number += step * (bit == '1' ? 1 : 0);
        step = step >> 1;

        if (step == 0) {
            file.write(reinterpret_cast<char*>(&number), sizeof(unsigned short));
            step = SHORT_MAX;
            number = 0;
        }
    }
}
