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
    ID,       // 标识符（变量名）
    EQUAL,    // =
    PLUS,     // +
    STAR,     // *
    LPAREN,   // (
    RPAREN,   // )
    END,      // 结束符
    INVALID   // 无效符号
};

// 定义符号到字符串的映射
map<Token, string> tokenToStr = {
    {NUM, "NUM"}, {ID, "ID"}, {EQUAL, "="}, {PLUS, "+"}, {STAR, "*"},
    {LPAREN, "("}, {RPAREN, ")"}, {END, "END"}, {INVALID, "INVALID"}
};

// 词法分析器
Token getToken(const string &input, size_t &pos, string &currentToken) {
    while (pos < input.length() && isspace(input[pos])) {
        ++pos;
    }
    if (pos >= input.length()) return END;

    if (isdigit(input[pos])) {  // 识别数字
        currentToken.clear();
        while (pos < input.length() && isdigit(input[pos])) {
            currentToken.push_back(input[pos]);
            ++pos;
        }
        return NUM;
    }
    if (isalpha(input[pos])) {  // 识别标识符（变量名）
        currentToken.clear();
        while (pos < input.length() && (isalnum(input[pos]) || input[pos] == '_')) {
            currentToken.push_back(input[pos]);
            ++pos;
        }
        return ID;
    }
    if (input[pos] == '=') {
        ++pos;
        currentToken = "=";
        return EQUAL;
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
        root = S();  // 从新的起始符号 S 开始推导并生成语法树
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

    void printRule(const string& rule) {
        cout << rule << endl;  // 输出推导过程
    }

    // 语法规则 S → id = E
    TreeNode* S() {
        TreeNode* node = new TreeNode("S", Token::INVALID);
        printRule("S -> id = E");

        // id
        consume(ID);
        node->addChild(new TreeNode("id", ID));

        // =
        consume(EQUAL);
        node->addChild(new TreeNode("=", EQUAL));

        // E
        TreeNode* eNode = E();
        node->addChild(eNode);

        return node;
    }

    // 语法规则 E → TE'
    TreeNode* E() {
        TreeNode* node = new TreeNode("E -> T E'", Token::INVALID);  // 非终结符 E
        TreeNode* tNode = T();
        node->addChild(tNode);
        TreeNode* ePrimeNode = E_();
        node->addChild(ePrimeNode);
        return node;
    }

    // 语法规则 E' → +TE' | ε
    TreeNode* E_() {
        if (token == PLUS) {
            TreeNode* node = new TreeNode("E' -> + T E'", Token::INVALID);
            printRule("E' -> + T E'");  // 打印推导过程
            consume(PLUS);
            TreeNode* tNode = T();
            node->addChild(new TreeNode("+", PLUS));  // 加号作为一个符号节点
            node->addChild(tNode);
            TreeNode* ePrimeNode = E_();
            node->addChild(ePrimeNode);
            return node;
        } else {
            TreeNode* node = new TreeNode("E' -> ε", Token::INVALID);
            printRule("E' -> ε");  // 打印推导过程
            return node;
        }
    }

    // 语法规则 T → FT'
    TreeNode* T() {
        TreeNode* node = new TreeNode("T -> F T'", Token::INVALID);  // 非终结符 T
        TreeNode* fNode = F();
        node->addChild(fNode);
        TreeNode* tPrimeNode = T_();
        node->addChild(tPrimeNode);
        return node;
    }

    // 语法规则 T' → *FT' | ε
    TreeNode* T_() {
        if (token == STAR) {
            TreeNode* node = new TreeNode("T' -> * F T'", Token::INVALID);
            printRule("T' -> * F T'");  // 打印推导过程
            consume(STAR);
            TreeNode* fNode = F();
            node->addChild(new TreeNode("*", STAR));  // 乘号作为一个符号节点
            node->addChild(fNode);
            TreeNode* tPrimeNode = T_();
            node->addChild(tPrimeNode);
            return node;
        } else {
            TreeNode* node = new TreeNode("T' -> ε", Token::INVALID);
            printRule("T' -> ε");  // 打印推导过程
            return node;
        }
    }

    // 语法规则 F → (E) | num | id
    TreeNode* F() {
        if (token == NUM) {
            string numStr = currentToken;  // 获取当前数字字符串
            string valueStr = "num:" + numStr;  // 创建 "num:1" 的标签
            TreeNode* node = new TreeNode("F", Token::INVALID);  // F 节点
            TreeNode* numNode = new TreeNode(valueStr, NUM);   // 创建实际数字值节点
            node->addChild(numNode);  // F 节点指向 num 节点
            printRule("F -> num:" + numStr);  // 打印推导过程
            consume(NUM);
            return node;
        } else if (token == ID) {
            string idStr = currentToken;
            string valueStr = "id:" + idStr;  // 创建 "id:variable" 的标签
            TreeNode* node = new TreeNode("F", Token::INVALID);  // F 节点
            TreeNode* idNode = new TreeNode(valueStr, ID);   // 创建标识符节点
            node->addChild(idNode);  // F 节点指向 id 节点
            printRule("F -> id:" + idStr);  // 打印推导过程
            consume(ID);
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
