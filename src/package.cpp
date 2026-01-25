#include "package.hxx"

std::set<ElementID> Package::assigned_ids_ = {};
std::set<ElementID> Package::freed_ids_ = {};

Package::Package() : id_([this]() {
    ElementID new_id;

    if (!freed_ids_.empty()) {
        new_id = *freed_ids_.begin();
        freed_ids_.erase(freed_ids_.begin());
    } else {
        if (assigned_ids_.empty()) {
            new_id = 1;
        } else {
            new_id = *assigned_ids_.rbegin() + 1;
        }
    }
    assigned_ids_.insert(new_id);
    return new_id;
}()) {}

Package::~Package() {
    assigned_ids_.erase(id_);
    freed_ids_.insert(id_);
}

Package& Package::operator=(Package&& package) noexcept {
    if (this == &package)
        return *this;
    assigned_ids_.erase(id_);
    freed_ids_.insert(id_);
    this->id_ = package.id_;
    assigned_ids_.insert(id_);
    return *this;
}