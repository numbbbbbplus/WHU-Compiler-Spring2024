/**
 * @file main.cpp
 * @brief GLSL Compiler for educational purposes
 * @date 2024-06-16
 * @license MIT License
 *
 * This program simulates a simple compiler that interprets and executes
 * commands from a .code file based on a custom language specification,
 * using input values from a .input file.
 *
 * @author techotaku@zs, WHU
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <memory>
#include <filesystem>

 // Token types enumeration: Defines the types of tokens in the source language
enum class TokenType {
    IDENTIFIER, NUMBER,
    ASSIGN, PRINT, INPUT,
    IF, THEN, ENDIF,
    COMPARE_OP, CALCULATE_OP,
    SEMICOLON, LPAREN, RPAREN,
    END
};

// Token structure: Represents a lexical token with a type and value
struct Token {
    TokenType type;
    std::string value;
};

// Lexer class: Tokenizes input source code
class Lexer {
public:
    explicit Lexer(const std::string& source) : sourceCode(source), position(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (position < sourceCode.size()) {
            char current = sourceCode[position];
            if (std::isspace(current)) {
                ++position;
            }
            else if (std::isalpha(current)) {
                tokens.push_back(readIdentifier());
            }
            else if (std::isdigit(current)) {
                tokens.push_back(readNumber());
            }
            else {
                tokens.push_back(readSingleCharToken(current));
            }
        }
        tokens.push_back({ TokenType::END, "" });
        return tokens;
    }

private:
    std::string sourceCode;
    size_t position;

    Token readIdentifier() {
        size_t start = position;
        while (position < sourceCode.size() && std::isalnum(sourceCode[position])) {
            ++position;
        }
        std::string value = sourceCode.substr(start, position - start);
        TokenType type = TokenType::IDENTIFIER;
        if (value == "print") type = TokenType::PRINT;
        else if (value == "input") type = TokenType::INPUT;
        else if (value == "if") type = TokenType::IF;
        else if (value == "then") type = TokenType::THEN;
        else if (value == "endif") type = TokenType::ENDIF;
        return { type, value };
    }

    Token readNumber() {
        size_t start = position;
        while (position < sourceCode.size() && (std::isdigit(sourceCode[position]) || sourceCode[position] == '.')) {
            ++position;
        }
        return { TokenType::NUMBER, sourceCode.substr(start, position - start) };
    }

    Token readSingleCharToken(char current) {
        switch (current) {
        case '=':
            return handleCompareOrAssign();
        case '>': case '<': case '!':
            return handleCompareOperator();
        case '+': case '-': case '*':
            ++position;
            return { TokenType::CALCULATE_OP, std::string(1, current) };
        case '(':
            ++position;
            return { TokenType::LPAREN, "(" };
        case ')':
            ++position;
            return { TokenType::RPAREN, ")" };
        case ';':
            ++position;
            return { TokenType::SEMICOLON, ";" };
        default:
            throw std::runtime_error("Unexpected character: " + std::string(1, current));
        }
    }

    Token handleCompareOrAssign() {
        if (sourceCode[position + 1] == '=') {
            position += 2;
            return { TokenType::COMPARE_OP, "==" };
        }
        ++position;
        return { TokenType::ASSIGN, "=" };
    }

    Token handleCompareOperator() {
        char op = sourceCode[position];
        ++position;
        if (sourceCode[position] == '=') {
            ++position;
            return { TokenType::COMPARE_OP, std::string(1, op) + "=" };
        }
        return { TokenType::COMPARE_OP, std::string(1, op) };
    }
};

// AST nodes: Define the structure of the AST, with each node type corresponding to constructs like statements and expressions.
struct ASTNode {
    virtual ~ASTNode() = default;
};

struct Expression : ASTNode {};
struct Statement : ASTNode {};

using ASTNodePtr = std::unique_ptr<ASTNode>;
using ExprPtr = std::unique_ptr<Expression>;
using StmtPtr = std::unique_ptr<Statement>;

struct Program : ASTNode {
    std::vector<StmtPtr> statements;
};

struct AssignStatement : Statement {
    std::string identifier;
    ExprPtr expression;
    AssignStatement(std::string id, ExprPtr expr)
        : identifier(std::move(id)), expression(std::move(expr)) {}
};

struct PrintStatement : Statement {
    ExprPtr expression;
    explicit PrintStatement(ExprPtr expr) : expression(std::move(expr)) {}
};

struct InputStatement : Statement {
    std::string identifier;
    explicit InputStatement(std::string id) : identifier(std::move(id)) {}
};

struct IfStatement : Statement {
    ExprPtr compareExpression;
    std::vector<StmtPtr> thenStatements;
    IfStatement(ExprPtr compExpr, std::vector<StmtPtr> thenStmts)
        : compareExpression(std::move(compExpr)), thenStatements(std::move(thenStmts)) {}
};

struct BinaryOperation : Expression {
    std::string op;
    ExprPtr left;
    ExprPtr right;
    BinaryOperation(std::string oper, ExprPtr lhs, ExprPtr rhs)
        : op(std::move(oper)), left(std::move(lhs)), right(std::move(rhs)) {}
};

struct Identifier : Expression {
    std::string name;
    explicit Identifier(std::string id) : name(std::move(id)) {}
};

struct Number : Expression {
    std::string value;
    explicit Number(std::string val) : value(std::move(val)) {}
};

// Parser class: Parses tokens into an AST
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens) : tokens(tokens), position(0) {}

    std::unique_ptr<Program> parse() {
        auto program = std::make_unique<Program>();
        while (currentToken().type != TokenType::END) {
            program->statements.push_back(parseStatement());
        }
        return program;
    }

private:
    const std::vector<Token>& tokens;
    size_t position;

    Token currentToken() const {
        return tokens[position];
    }

    Token consume(TokenType type) {
        if (currentToken().type != type) {
            throw std::runtime_error("Unexpected token: " + currentToken().value);
        }
        return tokens[position++];
    }

    // Parse a statement
    StmtPtr parseStatement() {
        if (currentToken().type == TokenType::IF) {
            return parseIfStatement();
        }
        else {
            auto stmt = parseSimpleStatement();
            consume(TokenType::SEMICOLON);
            return stmt;
        }
    }

    // Parse an if statement
    StmtPtr parseIfStatement() {
        consume(TokenType::IF);
        auto condition = parseExpression();
        consume(TokenType::THEN);
        std::vector<StmtPtr> thenStatements;
        while (currentToken().type != TokenType::ENDIF) {
            thenStatements.push_back(parseStatement());
        }
        consume(TokenType::ENDIF);
        consume(TokenType::SEMICOLON);
        return std::make_unique<IfStatement>(IfStatement{ std::move(condition), std::move(thenStatements) });
    }

    // Parse a simple statement
    StmtPtr parseSimpleStatement() {
        if (currentToken().type == TokenType::IDENTIFIER) {
            return parseAssignStatement();
        }
        else if (currentToken().type == TokenType::PRINT) {
            return parsePrintStatement();
        }
        else if (currentToken().type == TokenType::INPUT) {
            return parseInputStatement();
        }
        else {
            throw std::runtime_error("Unexpected simple statement");
        }
    }

    // Parse an assignment statement
    StmtPtr parseAssignStatement() {
        std::string identifier = consume(TokenType::IDENTIFIER).value;
        consume(TokenType::ASSIGN);
        auto expression = parseExpression();
        return std::make_unique<AssignStatement>(AssignStatement{ identifier, std::move(expression) });
    }

    // Parse a print statement
    StmtPtr parsePrintStatement() {
        consume(TokenType::PRINT);
        consume(TokenType::LPAREN);
        auto expression = parseExpression();
        consume(TokenType::RPAREN);
        return std::make_unique<PrintStatement>(PrintStatement{ std::move(expression) });
    }

    // Parse an input statement
    StmtPtr parseInputStatement() {
        consume(TokenType::INPUT);
        consume(TokenType::LPAREN);
        std::string identifier = consume(TokenType::IDENTIFIER).value;
        consume(TokenType::RPAREN);
        return std::make_unique<InputStatement>(InputStatement{ identifier });
    }

    // Parse an expression
    ExprPtr parseExpression() {
        auto left = parsePrimary();
        while (currentToken().type == TokenType::COMPARE_OP || currentToken().type == TokenType::CALCULATE_OP) {
            std::string op = consume(currentToken().type).value;
            auto right = parsePrimary();
            left = std::make_unique<BinaryOperation>(BinaryOperation{ op, std::move(left), std::move(right) });
        }
        return left;
    }

    // Parse a primary expression
    ExprPtr parsePrimary() {
        if (currentToken().type == TokenType::IDENTIFIER) {
            return std::make_unique<Identifier>(Identifier{ consume(TokenType::IDENTIFIER).value });
        }
        else if (currentToken().type == TokenType::NUMBER) {
            return std::make_unique<Number>(Number{ consume(TokenType::NUMBER).value });
        }
        else if (currentToken().type == TokenType::LPAREN) {
            consume(TokenType::LPAREN);
            auto expression = parseExpression();
            consume(TokenType::RPAREN);
            return expression;
        }
        else {
            throw std::runtime_error("Unexpected primary expression");
        }
    }
};

// Interpreter class: Executes the AST
class Interpreter {
public:
    Interpreter(Program* program, const std::vector<int>& inputs)
        : program(program), inputs(inputs), inputIndex(0) {}

    void interpret() {
        for (auto& statement : program->statements) {
            execute(statement.get());
        }
    }

private:
    Program* program;
    std::vector<int> inputs;
    size_t inputIndex;
    std::unordered_map<std::string, int> variables;

    // Execute a statement
    void execute(Statement* statement) {
        if (auto assignStmt = dynamic_cast<AssignStatement*>(statement)) {
            int value = evaluate(assignStmt->expression.get());
            variables[assignStmt->identifier] = value;
        }
        else if (auto printStmt = dynamic_cast<PrintStatement*>(statement)) {
            int value = evaluate(printStmt->expression.get());
            std::cout << value << std::endl;
        }
        else if (auto inputStmt = dynamic_cast<InputStatement*>(statement)) {
            variables[inputStmt->identifier] = inputs[inputIndex++];
        }
        else if (auto ifStmt = dynamic_cast<IfStatement*>(statement)) {
            if (evaluate(ifStmt->compareExpression.get())) {
                for (auto& stmt : ifStmt->thenStatements) {
                    execute(stmt.get());
                }
            }
        }
        else {
            throw std::runtime_error("Unexpected statement");
        }
    }

    // Evaluate an expression
    int evaluate(Expression* expression) {
        if (auto binOp = dynamic_cast<BinaryOperation*>(expression)) {
            int left = evaluate(binOp->left.get());
            int right = evaluate(binOp->right.get());
            if (binOp->op == "+") return left + right;
            if (binOp->op == "-") return left - right;
            if (binOp->op == "*") return left * right;
            if (binOp->op == ">") return left > right;
            if (binOp->op == "<") return left < right;
            if (binOp->op == "==") return left == right;
            if (binOp->op == "!=") return left != right;
            if (binOp->op == ">=") return left >= right;
            if (binOp->op == "<=") return left <= right;
            throw std::runtime_error("Unexpected binary operator: " + binOp->op);
        }
        else if (auto ident = dynamic_cast<Identifier*>(expression)) {
            return variables.at(ident->name);
        }
        else if (auto num = dynamic_cast<Number*>(expression)) {
            return std::stoi(num->value);
        }
        else {
            throw std::runtime_error("Unexpected expression");
        }
    }
};

// Main function: Entry point of the program
int main() {
    // Commented out for deployment; Uncomment for debugging purposes
    // std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;

    // Open code and input files
    // Note: Ensure that test.code and test.input are in the build directory when using cmake for compilation.
    std::ifstream codeFile("test.code");
    std::ifstream inputFile("test.input");

    // Check if files are opened successfully
    if (!codeFile) {
        std::cerr << "Error opening 'test.code'." << std::endl;
        return 1;
    }
    if (!inputFile) {
        std::cerr << "Error opening 'test.input'." << std::endl;
        return 1;
    }

    // Read code and input files
    std::string code((std::istreambuf_iterator<char>(codeFile)), std::istreambuf_iterator<char>());
    std::string inputLine;
    std::vector<int> inputs;

    while (std::getline(inputFile, inputLine)) {
        inputs.push_back(std::stoi(inputLine));
    }

    // Tokenize the code
    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    // Parse the tokens into an AST
    Parser parser(tokens);
    auto program = parser.parse();

    // Interpret the AST
    Interpreter interpreter(program.get(), inputs);
    interpreter.interpret();

    return 0;
}
