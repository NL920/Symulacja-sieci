#pragma once
#include <list>
#include <types.hxx>
#include <algorithm>
template<class T>
class NodeCollection {
public:

    using container_t = typename std::list<T>;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;


    iterator begin() {return container_.begin();}
    iterator end() {return container_.end();}
    const_iterator begin() const {return container_.cbegin();}
    const_iterator end() const {return container_.cend();}
    const_iterator cbegin() const {return container_.cbegin();}
    const_iterator cend() const {return container_.cend();}

    void add(T&& node) { container_.emplace_back(std::move(node)); }

    iterator find_by_id(ElementID id) {
        return std::find_if(container_.begin(), container_.end(),
            [id](const T& node) { return node.get_id() == id; });
    }

    const_iterator find_by_id(ElementID id) const {
        return std::find_if(container_.cbegin(), container_.cend(),
            [id](const T& node) { return node.get_id() == id; });
    }

    void remove_by_id(ElementID id) {
        auto it = find_by_id(id);
        if (it != container_.end()) {
            container_.erase(it);
        }
    }

private:
    container_t container_;

};