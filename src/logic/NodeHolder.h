#pragma once

#include <Node.hpp>

#include <functional>

namespace mlbb {

//! \brief Helper for storing nodes along with the info if it was used or not
template<class T>
class MarkableNode {
public:
    MarkableNode(T* node) : Node(node)
    {
        if(node == nullptr)
            throw std::runtime_error("Node is null in MarkableNode");
    }

    T* Node;
    bool Marked = true;
};

//! \brief Holds child nodes with easy access to some operations
template<class T>
class NodeHolder {
public:
    NodeHolder(godot::Node* container, std::function<T*()> createNew) :
        Container(container), CreateNew(createNew)
    {}

    NodeHolder(NodeHolder&& other) = default;
    NodeHolder& operator=(NodeHolder&& other) = default;

    NodeHolder(const NodeHolder& other) = delete;
    NodeHolder& operator=(const NodeHolder& other) = delete;

    void UnMarkAll()
    {
        for(auto& node : CreatedNodes) {
            node.Marked = false;
        }

        NextPos = 0;
    }

    void FreeUnmarked()
    {
        for(auto iter = CreatedNodes.begin(); iter != CreatedNodes.end();) {
            if(iter->Marked) {
                ++iter;
            } else {

                iter->Node->free();
                CreatedNodes.erase(iter);
            }
        }
    }

    T* GetNext()
    {
        if(NextPos >= CreatedNodes.size()) {
            auto node = CreateNew();
            Container->add_child(node);
            CreatedNodes.push_back(MarkableNode(node));
            ++NextPos;
            return node;
        }

        auto& existing = CreatedNodes[NextPos++];
        existing.Marked = true;
        return existing.Node;
    }

private:
    godot::Node* Container;
    std::function<T*()> CreateNew;

    std::vector<MarkableNode<T>> CreatedNodes;
    size_t NextPos = 0;
};
} // namespace mlbb
