#include <iostream>
#include <unistd.h>

#include "huffman.h"

void printHelp(char** argv) {
    std::cout << "Usage: " << argv[0] << " [options] [file]\n"
        << "Options:\n"
        << "  -c <file>    Encode the specified file\n"
        << "  -d <file>    Decode the specified file\n"
        << "  -h           Show this help message\n";
}

int main(const int argc, char** argv) {
    const int option = getopt(argc, argv, ":c:d:h");
    std::string fileName;
    switch (option) {
    case 'c':
        std::cout << "File to encode: " << optarg << std::endl;
        fileName = optarg;
        compression(fileName);
        return EXIT_SUCCESS;
    case 'd':
        std::cout << "File to decode: " << optarg << std::endl;
        fileName = optarg;
        decompression(fileName);
        return EXIT_SUCCESS;
    case 'h':
        printHelp(argv);
        return EXIT_SUCCESS;
    case '?':
        std::cerr << "Unknown option: " << static_cast<char>(optopt) << std::endl;
        printHelp(argv);
        return EXIT_FAILURE;
    case ':':
        std::cerr << "The file name is expected for option: " << std::endl;
        printHelp(argv);
        return EXIT_FAILURE;
    default:
        printHelp(argv);
        return EXIT_FAILURE;
    }
}
