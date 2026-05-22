#include "Lexer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

// --- Constructor: Read entire file into source string ---
Lexer::Lexer(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Lexer: cannot open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    source = buffer.str();
    pos  = 0;
    line = 1;
}

// --- Get current character ---
char Lexer::currentChar() const {
    if (pos >= (int)source.size()) return '\0';
    return source[pos];
}

// --- Look at next character without moving ---
char Lexer::peek() const {
    if (pos + 1 >= (int)source.size()) return '\0';
    return source[pos + 1];
}

// --- Move to next character ---
void Lexer::advance() {
    if (pos < (int)source.size()) {
        if (source[pos] == '\n') line++;
        pos++;
    }
}

bool Lexer::isLetter(char c) const {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z');
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isOperatorSymbol(char c) const {
    // From RPAL_Lex.pdf:
    // + - * < > & . @ / : = ~ | $ ! # % ^ _ [ ] { } " ' ?
    const std::string OPS = "+-*<>&.@/:=~|$!#%^_[]{}\"'?";
    return OPS.find(c) != std::string::npos;
}

// --- Read identifier or keyword ---
// RPAL_Lex.pdf: Identifier -> Letter (Letter | Digit | '_')*
Token Lexer::readIdentifierOrKeyword() {
    std::string result;
    // First char is always a letter (guaranteed by tokenize())
    while (pos < (int)source.size() &&
           (isLetter(currentChar()) ||
            isDigit(currentChar())  ||
            currentChar() == '_')) {
        result += currentChar();
        advance();
    }
    // Check keyword set AFTER reading the full word
    if (KEYWORDS.count(result)) {
        return Token(TokenType::KEYWORD, result);
    }
    return Token(TokenType::IDENTIFIER, result);
}

// --- Read integer ---
// RPAL_Lex.pdf: Integer -> Digit+ 
Token Lexer::readInteger() {
    std::string result;
    while (pos < (int)source.size() && isDigit(currentChar())) {
        result += currentChar();
        advance();
    }
    return Token(TokenType::INTEGER, result);
}

// --- Read string inside single quotes ---
// RPAL_Lex.pdf: String -> ' ( ( '\' . ) | ( Char - "'" ) )* '
Token Lexer::readString() {
    std::string result;
    result += '\'';      // include opening quote in value
    advance();           // skip opening quote
 
    while (pos < (int)source.size() && currentChar() != '\'') {
        if (currentChar() == '\\') {
            // Escape sequence
            advance(); // skip backslash
            if (pos < (int)source.size()) {
                char escaped = currentChar();
                // Valid escapes: \t \n \\ \'
                if (escaped == 't') {
                    result += '\t';
                    advance();
                } else if (escaped == 'n') {
                    result += '\n';
                    advance();
                } else if (escaped == '\\') {
                    result += '\\';
                    advance();
                } else if (escaped == '\'') {
                    result += '\'';
                    advance();
                } else {
                    // Invalid escape - include as-is and report warning
                    std::cerr << "Lexer warning: unknown escape sequence '\\"
                              << escaped << "' at line " << line << "\n";
                    result += '\\';
                    result += escaped;
                    advance();
                }
            }
        } else {
            result += currentChar();
            advance();
        }
    }
 
    if (currentChar() == '\'') {
        result += '\''; // include closing quote
        advance();      // skip closing quote
    } else {
        std::cerr << "Lexer error: unterminated string at line "
                  << line << "\n";
    }
    return Token(TokenType::STRING, result);
}

// --- Read operator symbols ---
// RPAL_Lex.pdf: Operator -> Operator_symbol+
Token Lexer::readOperator() {
    std::string result;
    while (pos < (int)source.size() &&
           isOperatorSymbol(currentChar()) &&
           currentChar() != '\'') {   // '\'' starts a string, not part of operator
        result += currentChar();
        advance();
    }
    return Token(TokenType::OPERATOR, result);
}

// --- Read spaces ---
// RPAL_Lex.pdf: Space -> ( Space | HT | EOL )  => <DELETE>
Token Lexer::readSpaces() {
    std::string result;
    while (pos < (int)source.size() &&
           (currentChar() == ' '  ||
            currentChar() == '\t' ||
            currentChar() == '\n' ||
            currentChar() == '\r')) {
        result += currentChar();
        advance();
    }
    return Token(TokenType::SPACES, result);
}

// --- Read comments ---
// RPAL_Lex.pdf: Comment -> '//' ( valid_chars )* Eol => <DELETE>
Token Lexer::readComment() {
    std::string result;
    // Consume the leading '//' (guaranteed by tokenize caller)
    result += currentChar(); advance(); // first '/'
    result += currentChar(); advance(); // second '/'
 
    // Read until end of line or EOF
    while (pos < (int)source.size() &&
           currentChar() != '\n' &&
           currentChar() != '\r') {
        result += currentChar();
        advance();
    }
    return Token(TokenType::COMMENT, result);
}

// --- Main tokenize function ---
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    tokens.reserve(512); // pre-allocate for performance
 
    while (pos < (int)source.size()) {
        char c = currentChar();
 
        // 1. Whitespace (space, tab, newline)
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            tokens.push_back(readSpaces());
            continue;
        }
 
        // 2. Comment ( // )
        //    Must check BEFORE operator - both start with '/'
        if (c == '/' && peek() == '/') {
            tokens.push_back(readComment());
            continue;
        }
 
        // 3. Identifier or keyword ( starts with a letter )
        if (isLetter(c)) {
            tokens.push_back(readIdentifierOrKeyword());
            continue;
        }
 
        // 4. Integer ( starts with a digit )
        if (isDigit(c)) {
            tokens.push_back(readInteger());
            continue;
        }
 
        // 5. String ( starts with single-quote )
        //    Must check BEFORE operator because '\'' is also an operator symbol
        if (c == '\'') {
            tokens.push_back(readString());
            continue;
        }
 
        // 6. Punctuation - exactly these four characters: ( ) , ;
        if (c == '(' || c == ')' || c == ',' || c == ';') {
            tokens.push_back(Token(TokenType::PUNCTUATION,
                                   std::string(1, c)));
            advance();
            continue;
        }
 
        // 7. Operator ( any operator symbol except '\'' which is handled above )
        if (isOperatorSymbol(c)) {
            tokens.push_back(readOperator());
            continue;
        }
 
        // 8. Unknown character - report warning, skip
        std::cerr << "Lexer warning: unknown character '"
                  << c << "' (ASCII " << (int)c
                  << ") at line " << line << "\n";
        advance();
    }
 
    // 9. Append the END_OF_FILE sentinel
    tokens.push_back(Token(TokenType::END_OF_FILE, "EOF"));
    return tokens;
}