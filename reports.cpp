#include "reports.hxx"
#include <algorithm>
#include <vector>
#include <iostream>

template <typename T>
std::vector<const T*> sort_nodes_by_id(auto begin, auto end) {
    std::vector<const T*> sorted;
    for (auto it = begin; it != end; ++it) {
        sorted.push_back(&(*it));
    }//sortownie po ID
    std::sort(sorted.begin(), sorted.end(), [](const T* a, const T* b) {
        return a->get_id() < b->get_id();
    });
    return sorted;
}

void generate_structure_report(const Factory& f, std::ostream& os) {
    os << "\n== LOADING RAMPS ==\n\n";
    auto ramps = sort_nodes_by_id<Ramp>(f.ramp_cbegin(), f.ramp_cend());

    for (const auto* r : ramps) {
        os << "LOADING RAMP #" << r->get_id() << "\n";
        os << "  Delivery interval: " << r->get_delivery_interval() << "\n";
        os << "  Receivers:\n";
        std::vector<ElementID> worker_ids;
        std::vector<ElementID> storehouse_ids;
        for (const auto& pair : r->receiver_preferences_.get_preferences()) {
            IPackageReceiver* receiver = pair.first;
            if (dynamic_cast<Storehouse*>(receiver)) {
                storehouse_ids.push_back(receiver->get_id());
            } else if (dynamic_cast<Worker*>(receiver)) {
                worker_ids.push_back(receiver->get_id());
            }
        }
        std::sort(storehouse_ids.begin(), storehouse_ids.end());
        std::sort(worker_ids.begin(), worker_ids.end());
        for (auto id : storehouse_ids) {
            os << "    storehouse #" << id << "\n";
        }
        for (auto id : worker_ids) {
            os << "    worker #" << id << "\n";
        }
        os << "\n";
    }
    os << "\n== WORKERS ==\n\n";

    auto workers = sort_nodes_by_id<Worker>(f.worker_cbegin(), f.worker_cend());
    for (const auto* w : workers) {
        os << "WORKER #" << w->get_id() << "\n";
        os << "  Processing time: " << w->get_processing_duration() << "\n";
        os << "  Queue type: FIFO\n";
        os << "  Receivers:\n";
        std::vector<ElementID> worker_ids;
        std::vector<ElementID> storehouse_ids;

        for (const auto& pair : w->receiver_preferences_.get_preferences()) {
            IPackageReceiver* receiver = pair.first;
            
            if (dynamic_cast<Storehouse*>(receiver)) {
                storehouse_ids.push_back(receiver->get_id());
            } else if (dynamic_cast<Worker*>(receiver)) {
                worker_ids.push_back(receiver->get_id());
            }
        }

        std::sort(storehouse_ids.begin(), storehouse_ids.end());
        std::sort(worker_ids.begin(), worker_ids.end());

        for (auto id : storehouse_ids) {
            os << "    storehouse #" << id << "\n";
        }
        for (auto id : worker_ids) {
            os << "    worker #" << id << "\n";
        }
        os << "\n";
    }
    os << "\n== STOREHOUSES ==\n\n";
    auto storehouses = sort_nodes_by_id<Storehouse>(f.storehouse_cbegin(), f.storehouse_cend());
    for (const auto* s : storehouses) {
        os << "STOREHOUSE #" << s->get_id() << "\n\n";
    }
}

void generate_simulation_turn_report(const Factory& f, std::ostream& os, Time t) {
    os << "=== [ Turn: " << t << " ] ===\n\n";
    os << "== WORKERS ==\n";
    auto workers = sort_nodes_by_id<Worker>(f.worker_cbegin(), f.worker_cend());
    bool is_first_worker = true;
    for (const auto* w : workers) {
        if (!is_first_worker) os << "\n";
        is_first_worker = false;
        os << "WORKER #" << w->get_id() << "\n";
        os << "  PBuffer: ";
        if (w->get_processing_buffer().has_value()) {
            Time progress = t - w->get_package_processing_start_time() + 1;
            os << "#" << w->get_processing_buffer()->get_id() << " (pt = " << progress << ")\n";
        } else {
            os << "(empty)\n";
        }
        os << "  Queue: ";
        if (w->cbegin() == w->cend()) {
            os << "(empty)\n";
        } else {
            bool first_element = true;
            for (auto it = w->cbegin(); it != w->cend(); ++it) {
                if (!first_element) os << ", ";
                os << "#" << it->get_id();
                first_element = false;
            }
            os << "\n";
        }
        os << "  SBuffer: ";
        if (w->get_sending_buffer().has_value()) {
            os << "#" << w->get_sending_buffer()->get_id() << "\n";
        } else {
            os << "(empty)\n";
        }
    }

    os << "\n\n== STOREHOUSES ==\n\n";
    auto storehouses = sort_nodes_by_id<Storehouse>(f.storehouse_cbegin(), f.storehouse_cend());
    for (const auto* s : storehouses) {
        os << "STOREHOUSE #" << s->get_id() << "\n";
        os << "  Stock: ";
        if (s->cbegin() == s->cend()) {
            os << "(empty)\n\n";
        } else {
            bool first_element = true;
            for (auto it = s->cbegin(); it != s->cend(); ++it) {
                if (!first_element) os << ", ";
                os << "#" << it->get_id();
                first_element = false;
            }
            os << "\n\n";
        }
    }
}