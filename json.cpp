#include "json.h"
 

 
using namespace std::literals;
 

 
namespace json {
 

 
namespace {
 

 
Node LoadNode(std::istream& input);
 

 
Node LoadNull(std::istream& input) { // OK
 
    const std::string name_null = "null";
 
    
 
    std::string line;
 
    while (isalpha(static_cast<unsigned char>(input.peek()))) {
 
        line.push_back(static_cast<char>(input.get()));
 
    }
 
    if (line == name_null) return {};
 
    else throw ParsingError("Bool parsing error");
 
}
 

 
Node LoadBool(std::istream& input) { // OK
 
    const std::string name_false = "false";
 
    const std::string name_true = "true";
 
    
 
    std::string line;
 
    while (isalpha(static_cast<unsigned char>(input.peek()))) {
 
        line.push_back(static_cast<char>(input.get()));
 
    }
 
    if (line == name_true) return Node(true);
 
    else if (line == name_false) return Node(false);
 
    else throw ParsingError("Bool parsing error");
 
}
 

 
Node LoadArray(std::istream& input) {
 
    Array result;
 

 
    for (char c; input >> c && c != ']';) {
 
        if (c != ',') {
 
            input.putback(c);
 
        }
 
        result.push_back(LoadNode(input));
 
    }
 
    if (!input) {
 
        throw ParsingError("Array parsing error"s);
 
    }
 
    return Node(std::move(result));
 
}
 

 
using Number = std::variant<double, int>;
 

 
Number LoadNumber(std::istream& input) {
 
    std::string parsed_num;
 

 
    // Считывает в parsed_num очередной символ из input
 
    auto read_char = [&parsed_num, &input] {
 
        parsed_num += static_cast<char>(input.get());
 
        if (!input) {
 
            throw ParsingError("Failed to read number from stream"s);
 
        }
 
    };
 

 
    // Считывает одну или более цифр в parsed_num из input
 
    auto read_digits = [&input, read_char] {
 
        if (!std::isdigit(input.peek())) {
 
            throw ParsingError("A digit is expected"s);
 
        }
 
        while (std::isdigit(input.peek())) {
 
            read_char();
 
        }
 
    };
 

 
    if (input.peek() == '-') {
 
        read_char();
 
    }
 
    // Парсим целую часть числа
 
    if (input.peek() == '0') {
 
        read_char();
 
        // После 0 в JSON не могут идти другие цифры
 
    } else {
 
        read_digits();
 
    }
 

 
    bool is_int = true;
 
    // Парсим дробную часть числа
 
    if (input.peek() == '.') {
 
        read_char();
 
        read_digits();
 
        is_int = false;
 
    }
 

 
    // Парсим экспоненциальную часть числа
 
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
 
        read_char();
 
        if (ch = input.peek(); ch == '+' || ch == '-') {
 
            read_char();
 
        }
 
        read_digits();
 
        is_int = false;
 
    }
 

 
    try {
 
        if (is_int) {
 
            // Сначала пробуем преобразовать строку в int
 
            try {
 
                return std::stoi(parsed_num);
 
            } catch (...) {
 
                // В случае неудачи, например, при переполнении,
 
                // код ниже попробует преобразовать строку в double
 
            }
 
        }
 
        return std::stod(parsed_num);
 
    } catch (...) {
 
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
 
    }
 
}
 

 
std::string LoadString(std::istream& input) {
 
    auto it = std::istreambuf_iterator<char>(input);
 
    auto end = std::istreambuf_iterator<char>();
 
    std::string s;
 
    while (true) {
 
        if (it == end) {
 
            // Поток закончился до того, как встретили закрывающую кавычку?
 
            throw ParsingError("String parsing error");
 
        }
 
        const char ch = *it;
 
        if (ch == '"') {
 
            // Встретили закрывающую кавычку
 
            ++it;
 
            break;
 
        } else if (ch == '\\') {
 
            // Встретили начало escape-последовательности
 
            ++it;
 
            if (it == end) {
 
                // Поток завершился сразу после символа обратной косой черты
 
                throw ParsingError("String parsing error");
 
            }
 
            const char escaped_char = *(it);
 
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
 
            switch (escaped_char) {
 
                case 'n':
 
                    s.push_back('\n');
 
                    break;
 
                case 't':
 
                    s.push_back('\t');
 
                    break;
 
                case 'r':
 
                    s.push_back('\r');
 
                    break;
 
                case '"':
 
                    s.push_back('"');
 
                    break;
 
                case '\\':
 
                    s.push_back('\\');
 
                    break;
 
                default:
 
                    // Встретили неизвестную escape-последовательность
 
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
 
            }
 
        } else if (ch == '\n' || ch == '\r') {
 
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
 
            throw ParsingError("Unexpected end of line"s);
 
        } else {
 
            // Просто считываем очередной символ и помещаем его в результирующую строку
 
            s.push_back(ch);
 
        }
 
        ++it;
 
    }
 

 
    return s;
 
}
 

 
Node LoadNum(std::istream& input) {
 
    auto result = LoadNumber(input);
 
    if (std::holds_alternative<int>(result)) return Node{std::get<int>(result)};
 
    else return Node{std::get<double>(result)};
 
}
 

 
Node LoadStr(std::istream& input) {
 
    auto result = LoadString(input);
 
    return Node{std::move(result)};
 
}
 

 
Node LoadDict(std::istream& input) {
 
    Dict result;
 

 
    for (char c; input >> c && c != '}';) {
 
        if (c == ',') {
 
            input >> c;
 
        }
 

 
        std::string key = LoadString(input);
 
        input >> c;
 
        result.insert({move(key), LoadNode(input)});
 
    }
 
    if (!input) {
 
        throw ParsingError("Dict parsing error"s);
 
    }
 
    return Node(move(result));
 
}
 

 
Node LoadNode(std::istream& input) {
 
    char c;
 
    input >> c;
 

 
    if (c == '[') {
 
        return LoadArray(input);
 
    } else if (c == '{') {
 
        return LoadDict(input);
 
    } else if (c == '"') {
 
        return LoadStr(input);
 
    } else if (c == 'n') {
 
        input.putback(c);
 
        return LoadNull(input);
 
    } else if ((c == 't') || (c == 'f')) {
 
        input.putback(c);
 
        return LoadBool(input);
 
    } else {
 
        input.putback(c);
 
        return LoadNum(input);
 
    }
 
}
 

 
}  // namespace
 

 
bool Node::IsInt() const {
 
    return std::holds_alternative<int>(value_);
 
}
 
bool Node::IsDouble() const {
 
    return (IsInt() || IsPureDouble());
 
}
 
bool Node::IsPureDouble() const {
 
    return std::holds_alternative<double>(value_);
 
}
 
bool Node::IsBool() const {
 
    return std::holds_alternative<bool>(value_);
 
}
 
bool Node::IsString() const {
 
    return std::holds_alternative<std::string>(value_);
 
}
 
bool Node::IsNull() const {
 
    return std::holds_alternative<std::nullptr_t>(value_);
 
}
 
bool Node::IsArray() const {
 
    return std::holds_alternative<Array>(value_);
 
}
 
bool Node::IsMap() const {
 
    return std::holds_alternative<Dict>(value_);
 
}
 

 
int Node::AsInt() const {
 
    if (!IsInt()) throw std::logic_error("wrong type Int");
 
    return std::get<int>(value_);
 
}
 
bool Node::AsBool() const {
 
    if (!IsBool()) throw std::logic_error("wrong type Bool");
 
    return std::get<bool>(value_);
 
}
 
double Node::AsDouble() const {
 
    if (IsPureDouble()) return std::get<double>(value_);
 
    else if (IsInt()) return static_cast<double>(std::get<int>(value_));
 
    else throw std::logic_error("wrong type Double");
 
}
 
const std::string& Node::AsString() const {
 
    if (!IsString()) throw std::logic_error("wrong type String");
 
    return std::get<std::string>(value_);
 
}
 
const Array& Node::AsArray() const {
 
    if (!IsArray()) throw std::logic_error("wrong type Array");
 
    return std::get<Array>(value_);
 
}
 
const Dict& Node::AsMap() const {
 
    if (!IsMap()) throw std::logic_error("wrong type Map");
 
    return std::get<Dict>(value_);
 
}
 

 
Document::Document(Node root)
 
    : root_(std::move(root)) {
 
}
 

 
const Node& Document::GetRoot() const {
 
    return root_;
 
}
 

 
Document Load(std::istream& input) {
 
    return Document{LoadNode(input)};
 
}
 

 
void PrintNode(const PrintContext& ctx, const Node& value);
 

 
void PrintValue(const PrintContext& ctx, const std::string& value) {
 
    ctx.out << "\""sv;
 
    for (const char c : value) {
 
        if (c == '\n') {
 
            ctx.out << "\\n"sv;
 
            continue;
 
        }
 
        if (c == '\r') {
 
            ctx.out << "\\r"sv;
 
            continue;
 
        }
 
        if (c == '"') ctx.out << "\\"sv;
 
        if (c == '\t') {
 
            ctx.out << "\\t"sv;
 
            continue;
 
        }
 
        if (c == '\\') ctx.out << "\\"sv;
 
        ctx.out << c;
 
    }
 
    ctx.out << "\""sv;
 
}
 

 
void PrintValue(const PrintContext& ctx, const std::nullptr_t) {
 
    ctx.out << "null"sv;
 
}
 

 
void PrintValue(const PrintContext& ctx, const bool value) {
 
    if (value) ctx.out << "true"sv;
 
    else ctx.out << "false"sv;
 
}
 

 
void PrintValue(const PrintContext& ctx, const Array& array) {
 
    ctx.out << "[\n"sv;
 
    auto ctx_indent = ctx.Indented();
 
    bool first = true;
 
    for (const auto& arr : array) {
 
        if (first) first = false;
 
        else ctx.out << ",\n"sv;
 
        ctx_indent.PrintIndent();
 
        PrintNode(arr, ctx_indent);
 
    }
 
    ctx.out << "\n"s;
 
    ctx.PrintIndent();
 
    ctx.out << "]"sv;
 
}
 

 
void PrintValue(const PrintContext& ctx, const Dict& dict) {
 
    ctx.out << "{\n"sv;
 
    auto ctx_indent = ctx.Indented();
 
    bool first = true;
 
    for (const auto& [key, value] : dict) {
 
        if (first) first = false;
 
        else ctx.out << ",\n";
 
        ctx_indent.PrintIndent();
 
        PrintValue(ctx, key);
 
        ctx.out << ": "; 
 
        PrintNode(value, ctx_indent);
 
    }
 
    ctx.out << "\n"s;
 
    ctx.PrintIndent();
 
    ctx.out << "}"sv;
 
}
 

 
void PrintNode(const Node& node, const PrintContext& ctx) {
 
    std::visit( [&ctx](const auto& value){ PrintValue(ctx, value); }, node.GetValue());
 
}
 

 
void Print(const Document& doc, std::ostream& out) {
 
    PrintNode(doc.GetRoot(), PrintContext{out});
 
}
 

 
}  // namespace json 