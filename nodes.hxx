#ifndef NETSIM_NODES_HPP
#define NETSIM_NODES_HPP
#include "types.hxx"
#include "storage_types.hxx"
#include "package.hxx"
#include "helpers.hxx"
#include <optional>
#include <memory>
#include <stdexcept>
#include <map>
#include <functional>
//potrafi przyjąć paczkę i iteruje po zawartosci
class IPackageReceiver {
public:
    using const_iterator = std::list<Package>::const_iterator;

    virtual ~IPackageReceiver() = default;
    virtual void receive_package(Package&& package) = 0;
    virtual ElementID get_id() const = 0;

    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
    virtual const_iterator cbegin() const = 0;
    virtual const_iterator cend() const = 0;
};

class ReceiverPreferences {
public:
    using preferences_t = std::map<IPackageReceiver*, double>;
    using const_iterator = preferences_t::const_iterator;

    explicit ReceiverPreferences(std::function<double()> pg = probability_generator);

    void add_receiver(IPackageReceiver* receiver);
    void remove_receiver(IPackageReceiver* receiver);
    IPackageReceiver* choose_receiver() const;

    const preferences_t& get_preferences() const;

    const_iterator begin() const { return prefs_.begin(); }
    const_iterator end() const { return prefs_.end(); }
    const_iterator cbegin() const { return prefs_.cbegin(); }
    const_iterator cend() const { return prefs_.cend(); }

private:
    preferences_t prefs_;
    std::function<double()> probability_generator_;
    void normalize_probabilities();
};

class PackageSender {
public:
    PackageSender() = default;
    PackageSender(PackageSender&&) = default;
    virtual ~PackageSender() = default;
    const std::optional<Package>& get_sending_buffer() const;
    void push_package(Package&& pkg);
    void send_package();
    std::optional<Package>& get_sending_buffer();

    ReceiverPreferences receiver_preferences_; 
protected:
    std::optional<Package> sending_buffer_;
};
class Ramp : public PackageSender {
public:
    Ramp(ElementID id, TimeOffset di);

    void deliver_goods(Time t);

    ElementID get_id() const { return id_; }
    TimeOffset get_delivery_interval() const { return delivery_interval_; }

private:
    ElementID id_;
    TimeOffset delivery_interval_;
};
using LoadingRamp = Ramp;

class Worker : public PackageSender, public IPackageReceiver {
public:
    Worker(ElementID id, TimeOffset pd, std::unique_ptr<IPackageQueue> q);

    void receive_package(Package&& pkg) override;
    void do_work(Time t);

    const_iterator begin() const override { return queue_->begin(); }
    const_iterator end() const override { return queue_->end(); }
    const_iterator cbegin() const override { return queue_->cbegin(); }
    const_iterator cend() const override { return queue_->cend(); }
    const std::optional<Package>& get_processing_buffer() const { return processed_package_; }
    ElementID get_id() const override { return id_; }

    TimeOffset get_processing_duration() const { return processing_duration_; }
    Time get_package_processing_start_time() const { return package_processing_start_time_; }
    std::optional<Package>& get_processing_buffer() { return processed_package_; }
private:
    ElementID id_;
    TimeOffset processing_duration_;
    std::unique_ptr<IPackageQueue> queue_;
    std::optional<Package> processed_package_;
    Time package_processing_start_time_;
};

class Storehouse : public IPackageReceiver {
public:
    Storehouse(ElementID id, std::unique_ptr<IPackageStockpile> d = std::make_unique<PackageQueue>(PackageQueueType::FIFO));

    void receive_package(Package&& package) override;
    ElementID get_id() const override { return id_; }

    const_iterator begin() const override { return stockpile_->begin(); }
    const_iterator end() const override { return stockpile_->end(); }
    const_iterator cbegin() const override { return stockpile_->cbegin(); }
    const_iterator cend() const override { return stockpile_->cend(); }

private:
    ElementID id_;
    std::unique_ptr<IPackageStockpile> stockpile_;
};

#endif