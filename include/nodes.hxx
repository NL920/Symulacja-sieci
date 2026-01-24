#pragma once

#include "types.hxx"
#include "storage_types.hxx"
#include "package.hxx"
#include "helpers.hxx"
#include <optional>
#include <memory>
#include <stdexcept>
#include <map>

// Miejsce klas pracownika, magazynu i rampy
class IPackageReceiver;
class ReceiverPreferences;

class PackageSender {
public:
    PackageSender() = default;
    PackageSender(PackageSender&&) = default;
    virtual ~PackageSender() = default;

    void push_package(Package&& pkg) {
        sending_buffer_ = std::move(pkg);
    }

    void send_package() {
        if (!sending_buffer_) return;

        IPackageReceiver* receiver = receiver_preferences.choose_receiver();
        if (!receiver) {
            throw std::runtime_error("PackageSender::send_package brak odbiorców");
        }

        receiver->receive_package(std::move(*sending_buffer_));
        sending_buffer_.reset();
    }

    std::optional<Package>& get_sending_buffer() { return sending_buffer_; }

    ReceiverPreferences receiver_preferences;  // publiczne pole

protected:
    std::optional<Package> sending_buffer_;
};


class ReceiverPreferences {
public:
    using preferences_t = std::map<IPackageReceiver*, double>;
    using const_iterator = preferences_t::const_iterator;

    ReceiverPreferences(std::function<double()> pg = probability_generator) 
        : probability_generator_(pg) {}

    // odbiorca i metody z nim związane
    void add_receiver(IPackageReceiver* receiver) {
        if (!receiver) return;
        prefs_[receiver] = 0.0;
        normalize_probabilities();
    }

    void remove_receiver(IPackageReceiver* receiver) {
        prefs_.erase(receiver);
        normalize_probabilities();
    }

    IPackageReceiver* choose_receiver() {
        if (prefs_.empty()) return nullptr;

        double p = probability_generator_();
        double cumulative = 0.0;
        for (auto& [receiver, prob] : prefs_) {
            cumulative += prob;
            if (p <= cumulative) return receiver;
        }
        return prefs_.rbegin()->first;
    }

    const preferences_t& get_preferences() const { return prefs_; }
    const_iterator begin() const { return prefs_.begin(); }
    const_iterator end() const { return prefs_.end(); }

private:
    preferences_t prefs_;
    std::function<double()> probability_generator_;

    void normalize_probabilities() {
        if (prefs_.empty()) return;
        double equal_prob = 1.0 / prefs_.size();
        for (auto& [_, prob] : prefs_) prob = equal_prob;
    }
};

//----stare
class Worker {
    public :
    Worker(WorkerID const ID,PackageQueueType const type) :
    id_(ID),
    qtype_(type), // Mozliwe ze informacja o tym jakiego typu jest pracownik LIFO czy FIFO moze byc uzyteczna
    queue_(type) {}
    ~Worker()=default;



    private :
    const WorkerID id_;
    PackageQueueType qtype_; // W zaleznosci od qtype pracownik bedzie bral poczatek lub koniec wektora kolejki.
    PackageQueue queue_;


};

class Warehouse {
    public :
    Warehouse(WarehouseID const ID) : id_(ID) {}
    ~Warehouse()=default;

    private :
    const WarehouseID id_;
    PackageStockpile queue_;
};