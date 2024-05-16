#include "json_builder.h"
 
namespace json{
    Builder::Builder() {
        if (nodes_stack_.empty()) {
            if (!root_.IsNull()) {
                throw std::logic_error("root has been added");
            } else{
                nodes_stack_.push_back(&root_);
            }
        }
    }
    
    Builder& Builder::Value(Node value_) {
        if (nodes_stack_.empty() || !root_.IsNull()) {
            //Вызов любого метода, кроме Build, при готовом объекте.
            throw std::logic_error("empty json");
        } else if (nodes_stack_.back()->IsArray()) {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(value_);
        } else {
            *nodes_stack_.back() = value_;
            nodes_stack_.erase(nodes_stack_.end() - 1);
        } 
        return *this;
    }
 
    Builder& Builder::Key(std::string key_) {
        if (nodes_stack_.empty() || !root_.IsNull()) {
            //Вызов любого метода, кроме Build, при готовом объекте.
            throw std::logic_error("empty json");
        }
        nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsMap())[key_]);
        return *this;
    }
 
    Builder &Builder::StartDict() {
        if (nodes_stack_.empty() || !root_.IsNull()) {
            //Вызов любого метода, кроме Build, при готовом объекте.
            throw std::logic_error("empty json");
        } else if (nodes_stack_.back()->IsArray()) {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Dict());
            Node* node = &const_cast<Array&>(nodes_stack_.back()->AsArray()).back();
            nodes_stack_.push_back(node);
        } else {
            *nodes_stack_.back() = Dict();
        }
        return *this;
    }
 
    Builder &Builder::StartArray() {
        if (nodes_stack_.empty() || !root_.IsNull()) {
            //Вызов любого метода, кроме Build, при готовом объекте.
            throw std::logic_error("empty json");
        } else if (nodes_stack_.back()->IsArray()) {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Array());
            Node* node = &const_cast<Array&>(nodes_stack_.back()->AsArray()).back();
            nodes_stack_.push_back(node);
        } else if(nodes_stack_.back()->IsNull()) {
            *nodes_stack_.back() = Array();
        }
        return *this;
    }
 
    Builder &Builder::EndArray() {
        if (nodes_stack_.empty() || !root_.IsNull()) {
            //Вызов любого метода, кроме Build, при готовом объекте.
            throw std::logic_error("empty json");
        }
        Node node = *nodes_stack_.back();
        if (!node.IsArray()) {
            //Вызов EndDict или EndArray в контексте другого контейнера.
            throw std::logic_error("object isn't dictionary");
        }
        *nodes_stack_.erase(nodes_stack_.end()-1);
        return *this;
    }
 
    Builder &Builder::EndDict() {
        if (nodes_stack_.empty() || !root_.IsNull()) {
            //Вызов любого метода, кроме Build, при готовом объекте.
            throw std::logic_error("empty json");
        }
        Node node = *nodes_stack_.back();
        if (!node.IsMap()) {
            //Вызов EndDict или EndArray в контексте другого контейнера.
            throw std::logic_error("object isn't dictionary");
        }
        *nodes_stack_.erase(nodes_stack_.end()-1);
        return *this;
    }
 
    Node Builder::Build() {
        if (root_.IsNull() || nodes_stack_.size() > 1) {
            //Вызов метода Build при неготовом описываемом объекте, то есть сразу после конструктора или при незаконченных массивах и словарях.
            throw std::logic_error("invalid json");
        }
        return std::move(root_);
    }
    
     Node Builder::GetNode(Node::Value value) {
        if (std::holds_alternative<int>(value)) return Node(std::get<int>(value));
        if (std::holds_alternative<double>(value)) return Node(std::get<double>(value));
        if (std::holds_alternative<std::string>(value)) return Node(std::get<std::string>(value));
        if (std::holds_alternative<std::nullptr_t>(value)) return Node(std::get<std::nullptr_t>(value));
        if (std::holds_alternative<bool>(value)) return Node(std::get<bool>(value));
        if (std::holds_alternative<Dict>(value)) return Node(std::get<Dict>(value));
        if (std::holds_alternative<Array>(value)) return Node(std::get<Array>(value));
        return {};
     }
}