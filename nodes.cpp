#include "nodes.hxx"
#include <algorithm>
#include <stdexcept>

ReceiverPreferences::ReceiverPreferences(std::function<double()> pg)
    : probability_generator_(pg) {}

void ReceiverPreferences::normalize_probabilities() {
    if (prefs_.empty()) return;
    double equal_prob = 1.0 / prefs_.size();
    for (auto& [_, prob] : prefs_) {
        prob = equal_prob;
    }
}
void ReceiverPreferences::add_receiver(IPackageReceiver* r) {
    if (!r) throw std::invalid_argument("nullptr");
    prefs_[r] = 0.0;
    normalize_probabilities();
}

void ReceiverPreferences::remove_receiver(IPackageReceiver* r) {
    auto it = prefs_.find(r);
    if (it == prefs_.end()) return;
    
    prefs_.erase(it);
    normalize_probabilities();
}

IPackageReceiver* ReceiverPreferences::choose_receiver() const {
    if (prefs_.empty()) return nullptr;

    double prob = probability_generator_();
    double distribution = 0.0;

    for (const auto& [receiver, probability] : prefs_) {
        distribution += probability;
        if (prob <= distribution) {
            return receiver;
        }
    }
    return prefs_.empty() ? nullptr : prefs_.begin()->first;
}

const ReceiverPreferences::preferences_t& ReceiverPreferences::get_preferences() const {
    return prefs_;
}
void PackageSender::push_package(Package&& package) {
    if (!sending_buffer_.has_value()) {
        sending_buffer_.emplace(std::move(package));
    }
}

void PackageSender::send_package() {
    if (sending_buffer_) {
        IPackageReceiver* receiver = receiver_preferences_.choose_receiver();
        if (receiver) {
            receiver->receive_package(std::move(*sending_buffer_));
            sending_buffer_.reset();
        }
    }
}
const std::optional<Package>& PackageSender::get_sending_buffer() const {
    return sending_buffer_;
}
std::optional<Package>& PackageSender::get_sending_buffer() {
    return sending_buffer_;
}
Worker::Worker(ElementID id, TimeOffset pd, std::unique_ptr<IPackageQueue> q)
    : id_(id), processing_duration_(pd), queue_(std::move(q)), 
      package_processing_start_time_(0) {}

void Worker::receive_package(Package&& p) {
    queue_->push(std::move(p));
}

void Worker::do_work(Time t) {
    if (!processed_package_ && !queue_->empty()) {
        processed_package_ = queue_->pop();
        package_processing_start_time_ = t;
    }

    if (processed_package_) {
        if (t - package_processing_start_time_ + 1 >= processing_duration_) {
            push_package(std::move(*processed_package_));
            processed_package_.reset();
        }
    }
}
Ramp::Ramp(ElementID id, TimeOffset di) 
    : id_(id), delivery_interval_(di) {}

void Ramp::deliver_goods(Time t) {
    if ((t - 1) % delivery_interval_ == 0) {
        push_package(Package());
    }
}
Storehouse::Storehouse(ElementID id, std::unique_ptr<IPackageStockpile> d)
    : id_(id), stockpile_(std::move(d)) {}

void Storehouse::receive_package(Package&& p) {
    stockpile_->push(std::move(p));
}