#include <iostream>
#include "lexer/Lexer.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./rpal20 <filename>" 
                  << std::endl;
        return 1;
    }

    Lexer lexer(argv[1]);
    auto tokens = lexer.tokenize();

    // Print all tokens to verify
    for (auto& token : tokens) {
        std::cout << "Type: " 
                  << (int)token.type 
                  << "  Value: " 
                  << token.value 
                  << std::endl;
    }

    return 0;
}