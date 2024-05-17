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
        if(nodes_stack_.empty()){
        if(!root_.IsNull()){
            throw std::logic_error("Value error: Дублирование Value в Root");
        }
        root_ = std::move(value_); 
    } else if (nodes_stack_.back()->IsArray()) {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(value_);
        } else {
            *nodes_stack_.back() = value_;
            nodes_stack_.erase(nodes_stack_.end() - 1);
        }  
        return *this;
    }
 
    DictKeyContext Builder::Key(std::string key_) {
        if (nodes_stack_.empty()) {
            throw std::logic_error("empty json");
        }
        nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsMap())[key_]);
        return *this;
    }
 
    DictItemContext Builder::StartDict() {
       if(nodes_stack_.empty()){
           if(!root_.IsNull()){
               throw std::logic_error("StartDict error: попытка дублирования Value в root");
           }
           root_ = Dict{};
           nodes_stack_.push_back(&root_);
       } else if (nodes_stack_.back()->IsArray()) {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Dict());
            Node* node = &const_cast<Array&>(nodes_stack_.back()->AsArray()).back();
            nodes_stack_.push_back(node);
        } else {
            *nodes_stack_.back() = Dict();
        }
        return *this;
    }
 
    ArrayItemContext Builder::StartArray() {
        if(nodes_stack_.empty()){
           if(!root_.IsNull()){
               throw std::logic_error("StartDict error: попытка дублирования Value в root");
           }
           root_ = Array();
           nodes_stack_.push_back(&root_);
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
        if (nodes_stack_.empty()) {
            throw std::logic_error("EndArray error: отсутствует открытый Array");
        } else if (!nodes_stack_.back()->IsArray()){
            throw std::logic_error("EndArray error: попытка закрыть Array при работе не с Array");
        } else if (nodes_stack_.size() == 1){
            root_ = *nodes_stack_.back();
        }
        nodes_stack_.pop_back();
        return *this;
    }
 
    Builder& Builder::EndDict(){
        if(nodes_stack_.empty()){
            throw std::logic_error("EndDict error: отсутствует открытый Dict");
        } else if (!nodes_stack_.back()->IsMap()){
            throw std::logic_error("EndDict error: попытка закрыть Dict при работе не с Dict");
        } else if (nodes_stack_.size() == 1){
            root_ = *nodes_stack_.back();
        }
        nodes_stack_.pop_back();
        return *this;
    }
 
    Node Builder::Build() {
        if (root_.IsNull()){
            throw std::logic_error("Build error: попытка создать пустой JSON");
        } else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()){
            throw std::logic_error("Build error: попытка создать JSON c открытым Array");
        } else if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap()){
            throw std::logic_error("Build error: попытка создать JSON c открытым Dict");
        } 
        return root_;
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
    
    DictItemContext::DictItemContext(Builder& builder)
        : builder_(builder)
    {}

    DictKeyContext DictItemContext::Key(std::string key) {
        return builder_.Key(key);
    }

    Builder& DictItemContext::EndDict() {
        return builder_.EndDict();
    }

    ArrayItemContext::ArrayItemContext(Builder& builder)
        : builder_(builder)
    {}

    ArrayItemContext ArrayItemContext::Value(Node::Value value) {
        return ArrayItemContext(builder_.Value(value));
    }

    DictItemContext ArrayItemContext::StartDict() {
        return builder_.StartDict();
    }

    ArrayItemContext ArrayItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& ArrayItemContext::EndArray() {
        return builder_.EndArray();
    }

    DictKeyContext::DictKeyContext(Builder& builder)
        : builder_(builder)
    {}

    DictItemContext DictKeyContext::Value(Node::Value value) {
        return DictItemContext(builder_.Value(value));
    }

    ArrayItemContext DictKeyContext::StartArray() {
        return builder_.StartArray();
    }

    DictItemContext DictKeyContext::StartDict() {
        return builder_.StartDict();
    }
}