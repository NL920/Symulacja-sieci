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

class Factory {

public:
    // RAMPY
    void add_ramp(Ramp&& r) {container_ramp_.add(std::move(r));}
    void remove_ramp(ElementID id) {container_ramp_.remove_by_id(id);}
    NodeCollection<Ramp>::iterator find_ramp_by_id(ElementID id) {return container_ramp_.find_by_id(id);}
    NodeCollection<Ramp>::const_iterator find_ramp_by_id(ElementID id) const {return container_ramp_.find_by_id(id);}
    NodeCollection<Ramp>::const_iterator ramp_cbegin() const {return container_ramp_.cbegin();}
    NodeCollection<Ramp>::const_iterator ramp_cend() const {return container_ramp_.cend();}

    // PRACOWNICY
    void add_worker(Worker&& w) {container_worker_.add(std::move(w));}
    void remove_worker(ElementID id);
    NodeCollection<Worker>::iterator find_worker_by_id(ElementID id) {return container_worker_.find_by_id(id);}
    NodeCollection<Worker>::const_iterator find_worker_by_id(ElementID id) const {return container_worker_.find_by_id(id);}
    NodeCollection<Worker>::const_iterator worker_cbegin() const {return container_worker_.cbegin();}
    NodeCollection<Worker>::const_iterator worker_cend() const {return container_worker_.cend();}

    // MAGAZYNY
    void add_storehouse(Storehouse&& s) {container_storehouse_.add(std::move(s));}
    void remove_storehouse(ElementID id);
    NodeCollection<Storehouse>::iterator find_storehouse_by_id(ElementID id) {return container_storehouse_.find_by_id(id);}
    NodeCollection<Storehouse>::const_iterator find_storehouse_by_id(ElementID id) const {return container_storehouse_.find_by_id(id);}
    NodeCollection<Storehouse>::const_iterator storehouse_cbegin() const {return container_storehouse_.cbegin();}
    NodeCollection<Storehouse>::const_iterator storehouse_cend() const {return container_storehouse_.cend();}


    bool is_consistent() const;
    void do_deliveries(Time);
    void do_package_passing();
    void do_work(Time);

private:

    template<class T>
    void remove_receiver(NodeCollection<T>& collection, ElementID id);

    NodeCollection<Ramp> container_ramp_;
    NodeCollection<Worker> container_worker_;
    NodeCollection<Storehouse> container_storehouse_;
};

