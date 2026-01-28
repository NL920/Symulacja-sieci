#pragma once

#include "types.hxx"
#include "storage_types.hxx"
#include "package.hxx"
#include "helpers.hxx"
#include <optional>
#include <memory>
#include <stdexcept>
#include <map>
#include <functional>

// Miejsce klas pracownika, magazynu i rampy

class IPackageReceiver {
public:
    using const_iterator = std::list<Package>::const_iterator;

    virtual ~IPackageReceiver() = default;

    virtual void receive_package(Package&& package) = 0;

    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
    
    virtual const_iterator cbegin() const = 0;
    virtual const_iterator cend() const = 0;

    virtual ElementID get_id() const = 0;
};

class ReceiverPreferences {
public:
    using preferences_t = std::map<IPackageReceiver*, double>;
    using const_iterator = preferences_t::const_iterator;

    ReceiverPreferences(std::function<double()> pg = probability_generator)
        : probability_generator_(pg) {}

    // odbiorca i metody z nim związane
    void add_receiver(IPackageReceiver* receiver) {
       if (!receiver) {
    throw std::invalid_argument("nullptr");
    }
        prefs_[receiver] = 0.0;
        normalize_probabilities();
    }

    void remove_receiver(IPackageReceiver* receiver) {
        prefs_.erase(receiver);
        normalize_probabilities();
    }

    IPackageReceiver* choose_receiver() const {
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

    preferences_t::iterator begin() { return prefs_.begin(); }
    preferences_t::iterator end() { return prefs_.end(); }

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

    IPackageReceiver* receiver = receiver_preferences_.choose_receiver();
    if (!receiver)return;

    receiver->receive_package(std::move(*sending_buffer_));
    sending_buffer_.reset();
    }


    std::optional<Package>& get_sending_buffer() { return sending_buffer_; }

    ReceiverPreferences receiver_preferences_;  

protected:
    std::optional<Package> sending_buffer_;
};




class Ramp : public PackageSender {
public:
    Ramp(ElementID id, TimeOffset di)
        : id_(id), delivery_interval_(di) {}

void deliver_goods(Time t) {
    if (t >= 1 && (t - 1) % delivery_interval_ == 0) {
        push_package(Package{});
    }
}

    ElementID get_id() const { return id_; }
    TimeOffset get_delivery_interval() const { return delivery_interval_; }

private:
    ElementID id_;
    TimeOffset delivery_interval_;
};

class Worker : public PackageSender, public IPackageReceiver {
public:
    using const_iterator = std::list<Package>::const_iterator;

    Worker(ElementID id, TimeOffset pd, std::unique_ptr<PackageQueue> q)
        : id_(id),
          processing_duration_(pd),
          queue_(std::move(q)),
          current_package_(std::nullopt),
          package_processing_start_time_(0) {}

    void receive_package(Package&& pkg) override {
        queue_->push(std::move(pkg));  //
    }

    void do_work(Time t) {
        if (!current_package_ && queue_->size() > 0) {
            current_package_.emplace(queue_->pop());
            package_processing_start_time_ = t;
        }
    
        if (current_package_ &&
            t - package_processing_start_time_ +1 >= processing_duration_) {

            push_package(std::move(*current_package_));
            current_package_.reset();
        }
    }

    const_iterator begin() const override { return queue_->begin(); }
    const_iterator end() const override { return queue_->end(); }
    const_iterator cbegin() const override { return begin(); }
    const_iterator cend() const override { return end(); }

    ElementID get_id() const override { return id_; }


    Time get_package_processing_start_time() const { return package_processing_start_time_; }
    TimeOffset get_processing_duration() const { return processing_duration_; }

private:
    ElementID id_;
    TimeOffset processing_duration_;
    std::unique_ptr<PackageQueue> queue_;
    std::optional<Package> current_package_;
    Time package_processing_start_time_;
};

class Storehouse : public IPackageReceiver {
public:
    Storehouse(ElementID id,
               std::unique_ptr<PackageQueue> d =
                   std::make_unique<PackageQueue>(PackageQueueType::FIFO))
        : id_(id), stockpile_(std::move(d)) {}

    void receive_package(Package&& package) override {
        stockpile_->push(std::move(package));
    }

    ElementID get_id() const override { return id_; }

    using const_iterator = PackageQueue::const_iterator;

    const_iterator begin() const override { return stockpile_->begin(); }
    const_iterator end() const override { return stockpile_->end(); }
    const_iterator cbegin() const override { return begin(); }
    const_iterator cend() const override { return end(); }
private:
    ElementID id_;
    std::unique_ptr<PackageQueue> stockpile_;
};
