# Huffman Encoding CLI Tool

## Description
This project is a command-line tool for encoding and decoding `.txt` files using Huffman coding. It provides options to encode a file into a compressed format and decode it back to its original form.

## Installation
1. Clone the repository:
    ```sh
    git clone https://github.com/Toudonou/huffman-encoding-cli.git
    ```
2. Navigate to the project directory:
    ```sh
    cd huffman-encoding-cli
    ```
3. Build the project using `g++`:
    ```sh
    g++ -std=c++20 main.cpp huffman.cpp -o huffman
    ```

## Usage
To use the CLI tool, run the executable with the appropriate options:

- Encode a `.txt` file:
    ```sh
    ./huffman -c <file.txt>
    ```
- Decode a `.thc` file:
    ```sh
    ./huffman -d <file.thc>
    ```
- Display the help message:
    ```sh
    ./huffman -h
    ```
## Author
- [Toudonou](https://github.com/Toudonou)


## License
This project is open source, under the MIT License [see](LICENSE).