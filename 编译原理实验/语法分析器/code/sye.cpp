#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <cctype>
#include <fstream>

using namespace std;

// 定义符号类型
enum Token {
    NUM,      // 数字
    PLUS,     // +
    STAR,     // *
    LPAREN,   // (
    RPAREN,   // )
    END,      // 结束符
    INVALID   // 无效符号
};

// 定义符号到字符串的映射
map<Token, string> tokenToStr = {
    {NUM, "NUM"}, {PLUS, "+"}, {STAR, "*"},
    {LPAREN, "("}, {RPAREN, ")"}, {END, "END"}, {INVALID, "INVALID"}
};

// 词法分析器
Token getToken(const string &input, size_t &pos, string &currentToken) {
    while (pos < input.length() && isspace(input[pos])) {
        ++pos;
    }
    if (pos >= input.length()) return END;

    if (isdigit(input[pos])) {
        currentToken.clear();
        while (pos < input.length() && isdigit(input[pos])) {
            currentToken.push_back(input[pos]);
            ++pos;
        }
        return NUM;
    }
    if (input[pos] == '+') {
        ++pos;
        currentToken = "+";
        return PLUS;
    }
    if (input[pos] == '*') {
        ++pos;
        currentToken = "*";
        return STAR;
    }
    if (input[pos] == '(') {
        ++pos;
        currentToken = "(";
        return LPAREN;
    }
    if (input[pos] == ')') {
        ++pos;
        currentToken = ")";
        return RPAREN;
    }
    return INVALID;
}

// 语法树节点定义
struct TreeNode {
    string value;         // 节点的值，包含符号或数字
    Token token;          // 终结符或非终结符
    vector<TreeNode*> children;

    TreeNode(const string& val, Token t) : value(val), token(t) {}

    void addChild(TreeNode* child) {
        children.push_back(child);
    }

    // 生成 DOT 格式的描述
    void toDot(string& dotStr, string parentId = "") const {
        static int idCounter = 0;
        stringstream ss;
        string nodeId = "node" + to_string(idCounter++);
        ss << nodeId << " [label=\"" << value << "\"];\n";  // 在节点中添加符号

        if (!parentId.empty()) {
            ss << parentId << " -> " << nodeId << ";\n";  // 添加父子连接
        }

        dotStr += ss.str();

        // 递归生成子节点的 DOT 描述
        for (const auto* child : children) {
            child->toDot(dotStr, nodeId);
        }
    }
};

// 解析器类
class LL1Parser {
public:
    LL1Parser(const string &expr) : input(expr), pos(0), token(END) {
        currentToken = "";
        token = getToken(input, pos, currentToken);  // 初始化当前符号
        root = nullptr; // 初始化根节点为空
    }

    void parse() {
        root = E();  // 从表达式开始推导并生成语法树
        string dotStr = "digraph G {\n";
        root->toDot(dotStr);  // 将语法树转换为 DOT 格式
        dotStr += "}\n";

        // 输出 DOT 文件
        ofstream outFile("syntax_tree.dot");
        outFile << dotStr;
        outFile.close();

        // 使用 Graphviz 的 dot 命令将 DOT 文件转换为 PNG 图像
        system("dot -Tpng syntax_tree.dot -o syntax_tree.png");

        cout << "Syntax tree image saved as 'syntax_tree.png'" << endl;
    }

private:
    string input;
    size_t pos;
    Token token;
    string currentToken; // 当前词法单元（数字、运算符等）
    TreeNode* root;  // 语法树的根节点

    void consume(Token expectedToken) {
        if (token == expectedToken) {
            token = getToken(input, pos, currentToken);
        } else {
            cout << "Error: Expected " << tokenToStr[expectedToken] << ", found " << tokenToStr[token] << endl;
            exit(1);
        }
    }

    TreeNode* E() {
        TreeNode* node = new TreeNode("E", Token::INVALID);  // 非终结符 E
        TreeNode* tNode = T();
        node->addChild(tNode);
        TreeNode* ePrimeNode = E_();
        node->addChild(ePrimeNode);
        return node;
    }

    TreeNode* E_() {
        if (token == PLUS) {
            TreeNode* node = new TreeNode("E' -> + T E'", Token::INVALID);  // 非终结符 E'
            consume(PLUS);
            TreeNode* tNode = T();
            node->addChild(new TreeNode("+", PLUS)); // 加号作为一个符号节点
            node->addChild(tNode);
            TreeNode* ePrimeNode = E_();
            node->addChild(ePrimeNode);
            return node;
        } else {
            return new TreeNode("E' -> ε", Token::INVALID);
        }
    }

    TreeNode* T() {
        TreeNode* node = new TreeNode("T", Token::INVALID);  // 非终结符 T
        TreeNode* fNode = F();
        node->addChild(fNode);
        TreeNode* tPrimeNode = T_();
        node->addChild(tPrimeNode);
        return node;
    }

    TreeNode* T_() {
        if (token == STAR) {
            TreeNode* node = new TreeNode("T' -> * F T'", Token::INVALID);  // 非终结符 T'
            consume(STAR);
            TreeNode* fNode = F();
            node->addChild(new TreeNode("*", STAR)); // 乘号作为一个符号节点
            node->addChild(fNode);
            TreeNode* tPrimeNode = T_();
            node->addChild(tPrimeNode);
            return node;
        } else {
            return new TreeNode("T' -> ε", Token::INVALID);
        }
    }

    TreeNode* F() {
        if (token == NUM) {
            string numStr = currentToken;  // 获取当前数字字符串
            string text = "F->num:";
            numStr=text + numStr;
            TreeNode* node = new TreeNode(numStr, NUM);  // 直接使用数字值作为节点值
            consume(NUM);
            return node;
        } else if (token == LPAREN) {
            TreeNode* node = new TreeNode("F -> ( E )", Token::INVALID);  // 非终结符 F

            // 左括号节点
            consume(LPAREN);
            node->addChild(new TreeNode("(", LPAREN));

            // 子节点 E
            TreeNode* eNode = E();
            node->addChild(eNode);

            // 右括号节点
            consume(RPAREN);
            node->addChild(new TreeNode(")", RPAREN));

            return node;
        } else {
            cout << "Error: Invalid token " << tokenToStr[token] << " in F" << endl;
            exit(1);
        }
    }
};

int main() {
    string expression;
    cout << "Enter expression: ";
    getline(cin, expression);

    LL1Parser parser(expression);
    parser.parse();

    return 0;
}
