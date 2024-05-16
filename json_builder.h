#include "json.h"

namespace json {
    
class Builder {
public:
    Builder();
    
    Builder& Key(std::string key_);
    Builder& Value(Node value_);
    Builder& StartDict();
    Builder& StartArray();   
    Builder& EndDict();   
    Builder& EndArray();
    
    json::Node Build();
    Node GetNode(Node::Value value);    
    
private:
    Node root_;
    std::vector<Node*> nodes_stack_;
};
    
} //namespace json