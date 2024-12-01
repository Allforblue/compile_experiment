#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <unordered_map>
#include <unordered_set>

enum class TokenType {
    KEYWORD,
    IDENTIFIER,
    NUMBER,
    SYMBOL,
    UNKNOWN,
    END
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
public:
    Lexer(const std::string& src) : source(src), index(0) {
        // 初始化关键字和符号
        keywords = {
            {"if", TokenType::KEYWORD},
            {"else", TokenType::KEYWORD},
            {"while", TokenType::KEYWORD},
            {"return", TokenType::KEYWORD}
        };

        symbols = { '+', '-', '*', '/', '=', ';', '(', ')', '{', '}' };
    }

    Token nextToken() {
        while (index < source.size() && std::isspace(source[index])) {
            index++; // Skip whitespace
        }

        if (index >= source.size()) {
            return { TokenType::END, "" };
        }

        char current = source[index];

        // Handle keywords and identifiers
        if (std::isalpha(current)) {
            std::string identifier;
            while (index < source.size() && (std::isalnum(source[index]) || source[index] == '_')) {
                identifier += source[index++];
            }
            if (keywords.find(identifier) != keywords.end()) {
                return { TokenType::KEYWORD, identifier };
            }
            return { TokenType::IDENTIFIER, identifier };
        }

        // Handle numbers
        if (std::isdigit(current)) {
            std::string number;
            while (index < source.size() && std::isdigit(source[index])) {
                number += source[index++];
            }
            return { TokenType::NUMBER, number };
        }

        // Handle symbols
        if (symbols.find(current) != symbols.end()) {
            index++;
            return { TokenType::SYMBOL, std::string(1, current) };
        }

        // Unknown token
        index++;
        return { TokenType::UNKNOWN, std::string(1, current) };
    }

private:
    std::string source;
    size_t index;
    std::unordered_map<std::string, TokenType> keywords;
    std::unordered_set<char> symbols; // 定义符号集
};

int main() {
    std::string code = "if (x == 10) { return x + 2; } else { return 0; } &";
    Lexer lexer(code);
    Token token;

    do {
        token = lexer.nextToken();
        std::cout << "Token: " << token.value << ", Type: ";
        switch (token.type) {
            case TokenType::KEYWORD: std::cout << "KEYWORD"; break;
            case TokenType::IDENTIFIER: std::cout << "IDENTIFIER"; break;
            case TokenType::NUMBER: std::cout << "NUMBER"; break;
            case TokenType::SYMBOL: std::cout << "SYMBOL"; break;
            case TokenType::UNKNOWN: std::cout << "UNKNOWN"; break;
            case TokenType::END: std::cout << "END"; break;
        }
        std::cout << std::endl;
    } while (token.type != TokenType::END);

    return 0;
}