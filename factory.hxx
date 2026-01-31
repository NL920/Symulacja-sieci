#ifndef FACTORY_HXX
#define FACTORY_HXX
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <iterator>
#include "nodes.hxx"
#include "types.hxx"



enum class ElementType { RAMP, WORKER, STOREHOUSE, LINK };

struct ParsedLineData {
    ElementType type;
    std::map<std::string, std::string> params;
};

ElementType parse_element_type(const std::string& token);
ParsedLineData parse_line(const std::string& line);


//to jest klasa pomocnicza  do testów no i pozwala iterować po mapie tak, jakby to była kolekcja obiektów yyy ukrywa pare i ptr
template <typename Node>
class NodeCollection {
public:
    using container_t = std::map<ElementID, std::unique_ptr<Node>>;

    class ConstIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Node;
        using difference_type = std::ptrdiff_t;
        using pointer = const Node*;
        using reference = const Node&;

        explicit ConstIterator(typename container_t::const_iterator it) : it_(it) {}
        reference operator*() const { return *(it_->second); }
        pointer operator->() const { return it_->second.get(); }
        ConstIterator& operator++() { ++it_; return *this; }
        ConstIterator operator++(int) { ConstIterator tmp = *this; ++it_; return tmp; }
        bool operator==(const ConstIterator& other) const { return it_ == other.it_; }
        bool operator!=(const ConstIterator& other) const { return it_ != other.it_; }
    private:
        typename container_t::const_iterator it_;
    };

    using const_iterator = ConstIterator;

    NodeCollection(const container_t& container) : container_(container) {}
    const_iterator cbegin() const { return ConstIterator(container_.cbegin()); }
    const_iterator cend() const { return ConstIterator(container_.cend()); }
    const_iterator begin() const { return cbegin(); }
    const_iterator end() const { return cend(); }

private:
    const container_t& container_;
};

class Factory {
public:
    void add_ramp(Ramp&& ramp) { ramps_.emplace(ramp.get_id(), std::make_unique<Ramp>(std::move(ramp))); }
    void add_worker(Worker&& worker) { workers_.emplace(worker.get_id(), std::make_unique<Worker>(std::move(worker))); }
    void add_storehouse(Storehouse&& storehouse) { storehouses_.emplace(storehouse.get_id(), std::make_unique<Storehouse>(std::move(storehouse))); }

    Ramp* find_ramp_by_id(ElementID id);
    Worker* find_worker_by_id(ElementID id);
    Storehouse* find_storehouse_by_id(ElementID id);

    // Iteratory dla testów-----zwracają wrapper NodeCollection
    NodeCollection<Ramp> ramps() const { return NodeCollection<Ramp>(ramps_); }
    NodeCollection<Worker> workers() const { return NodeCollection<Worker>(workers_); }
    NodeCollection<Storehouse> storehouses() const { return NodeCollection<Storehouse>(storehouses_); }

    auto ramp_cbegin() const { return ramps().cbegin(); }
    auto ramp_cend() const { return ramps().cend(); }
    auto worker_cbegin() const { return workers().cbegin(); }
    auto worker_cend() const { return workers().cend(); }
    auto storehouse_cbegin() const { return storehouses().cbegin(); }
    auto storehouse_cend() const { return storehouses().cend(); }

    bool is_consistent() const;
    void load_factory_structure(std::istream& is);
    void save_factory_structure(std::ostream& os) const;

private:
    std::map<ElementID, std::unique_ptr<Ramp>> ramps_;
    std::map<ElementID, std::unique_ptr<Worker>> workers_;
    std::map<ElementID, std::unique_ptr<Storehouse>> storehouses_;
};
Factory load_factory_structure(std::istream& is);
#endif