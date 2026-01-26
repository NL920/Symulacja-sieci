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
    //dodane remove_worker i remove_storehouse
    // PRACOWNICY
    void add_worker(Worker&& w) {container_worker_.add(std::move(w));}
    void remove_worker(ElementID id) {remove_receiver(container_worker_, id);}
    NodeCollection<Worker>::iterator find_worker_by_id(ElementID id) {return container_worker_.find_by_id(id);}
    NodeCollection<Worker>::const_iterator find_worker_by_id(ElementID id) const {return container_worker_.find_by_id(id);}
    NodeCollection<Worker>::const_iterator worker_cbegin() const {return container_worker_.cbegin();}
    NodeCollection<Worker>::const_iterator worker_cend() const {return container_worker_.cend();}

    // MAGAZYNY
    void add_storehouse(Storehouse&& s) {container_storehouse_.add(std::move(s));}
    void remove_storehouse(ElementID id) {remove_receiver(container_storehouse_, id);}
    NodeCollection<Storehouse>::iterator find_storehouse_by_id(ElementID id) {return container_storehouse_.find_by_id(id);}
    NodeCollection<Storehouse>::const_iterator find_storehouse_by_id(ElementID id) const {return container_storehouse_.find_by_id(id);}
    NodeCollection<Storehouse>::const_iterator storehouse_cbegin() const {return container_storehouse_.cbegin();}
    NodeCollection<Storehouse>::const_iterator storehouse_cend() const {return container_storehouse_.cend();}


    bool is_consistent() const { //tutaj trzeba sprawdzić
    std::vector<const IPackageReceiver*> all_receivers;
    for (const auto& worker : container_worker_)
        all_receivers.push_back(&worker);
    for (const auto& storehouse : container_storehouse_)
        all_receivers.push_back(&storehouse);

    for (const auto& ramp : container_ramp_) {
        for (auto& [receiver, _] : ramp.receiver_preferences.get_preferences()) {
            if (std::find(all_receivers.begin(), all_receivers.end(), receiver) == all_receivers.end())
                return false; 
        }
    }

    for (const auto& worker : container_worker_) {
        for (auto& [receiver, _] : worker.receiver_preferences.get_preferences()) {
            if (std::find(all_receivers.begin(), all_receivers.end(), receiver) == all_receivers.end())
                return false; 
        }
    }

    auto can_reach_storehouse = [this](const IPackageReceiver* start) -> bool {
        std::vector<const IPackageReceiver*> visited;
        std::vector<const IPackageReceiver*> stack{start};

        while (!stack.empty()) {
            const auto* current = stack.back();
            stack.pop_back();

            if (dynamic_cast<const Storehouse*>(current)) return true;

            if (std::find(visited.begin(), visited.end(), current) != visited.end()) continue;
            visited.push_back(current);

            const auto* sender_ptr = dynamic_cast<const PackageSender*>(current);
            if (sender_ptr) {
                for (auto& [receiver, _] : sender_ptr->receiver_preferences.get_preferences())
                    stack.push_back(receiver);
            }
        }
        return false; 
    };

    for (const auto& ramp : container_ramp_) {
        if (!can_reach_storehouse(&ramp)) return false;
    }

    return true;
}

    void do_deliveries(Time t) { //dodałam
        for (auto& ramp : container_ramp_) {
            ramp.deliver_goods(t);
        }
    }

    void do_package_passing() {
        for (auto& ramp : container_ramp_) {
            ramp.send_package();
        }

        for (auto& worker : container_worker_) {
            worker.send_package();
        }
    }

    void do_work(Time t) {
    for (auto& worker : container_worker_) {
            worker.do_work(t);
        }
    }

private:

    template<class T>
    void remove_receiver(NodeCollection<T>& collection, ElementID id) {//dodałam
    auto it = collection.find_by_id(id);
    if (it == collection.end()) return;
    IPackageReceiver* receiver_ptr = &(*it);// czy lepiej T* receiver_ptr = &(*it);
    for (auto& ramp : container_ramp_) {
        ramp.receiver_preferences.remove_receiver(receiver_ptr);
    }

    for (auto& worker : container_worker_) {
        worker.receiver_preferences.remove_receiver(receiver_ptr);
    }

    collection.remove_by_id(id);
    }
    
    NodeCollection<Ramp> container_ramp_;
    NodeCollection<Worker> container_worker_;
    NodeCollection<Storehouse> container_storehouse_;
};

