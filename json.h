#pragma once
 

 
#include <iostream>
 
#include <map>
 
#include <string>
 
#include <typeinfo>
 
#include <variant>
 
#include <vector>
 

 
namespace json {
 

 
class Node;
 
// Сохраните объявления Dict и Array без изменения
 
using Dict = std::map<std::string, Node>;
 
using Array = std::vector<Node>;
 

 
// Эта ошибка должна выбрасываться при ошибках парсинга JSON
 
class ParsingError : public std::runtime_error {
 
public:
 
    using runtime_error::runtime_error;
 
};
 

 
class Node {
 
public:
 
   /* Реализуйте Node, используя std::variant */
 

 
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
 
    
 
    template<typename T>
 
    Node(T value)
 
        : value_(std::move(value))
 
    {}
 
    
 
    Node()
 
        : value_(nullptr)
 
    {}
 
    
 
    bool IsInt() const;
 
    bool IsDouble() const;
 
    bool IsPureDouble() const;
 
    bool IsBool() const;
 
    bool IsString() const;
 
    bool IsNull() const;
 
    bool IsArray() const;
 
    bool IsMap() const;
 

 
    int AsInt() const;
 
    bool AsBool() const;
 
    double AsDouble() const;
 
    const std::string& AsString() const;
 
    const Array& AsArray() const;
 
    const Dict& AsMap() const;
 

 
    const Value& GetValue() const { return value_; }
 

 
    bool operator==(const Node& rhs) const { return value_ == rhs.value_; }
 
    bool operator!=(const Node& rhs) const { return !(value_ == rhs.value_); }
 

 
private:
 
    Value value_;
 
};
 

 
class Document {
 
public:
 
    explicit Document(Node root);
 
    Document() = default;
 
    const Node& GetRoot() const;
 

 
    bool operator==(const Document& rhs) {
 
        return root_ == rhs.root_;
 
    }   
 
    bool operator!=(const Document& rhs) {
 
        return !(root_ == rhs.root_);
 
    }
 

 
private:
 
    Node root_;
 
};
 

 
Document Load(std::istream& input);
 

 
struct PrintContext {
 
    std::ostream& out;
 
    int indent_step = 4;
 
    int indent = 0;
 

 
    void PrintIndent() const {
 
        for (int i = 0; i < indent; ++i) {
 
            out.put(' ');
 
        }
 
    }
 
    PrintContext Indented() const {
 
        return {out, indent_step, indent_step + indent};
 
    }
 
};
 

 
void PrintValue(const PrintContext& ctx, const std::string& value);
 
void PrintValue(const PrintContext& ctx, const std::nullptr_t);
 
void PrintValue(const PrintContext& ctx, const bool value);
 
void PrintValue(const PrintContext& ctx, const Array& array);
 
void PrintValue(const PrintContext& ctx, const Dict& dict);
 

 
template <typename Value>
 
void PrintValue(const PrintContext& ctx, const Value& value) {
 
    ctx.out << value;
 
}
 

 
void PrintNode(const Node& node, const PrintContext& ctx);
 
void Print(const Document& doc, std::ostream& output);
 

 

 

 
}  // namespace json 