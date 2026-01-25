// Klasa odpowiedzialna za polprodukt
#pragma once
#include <set>
#include "types.hxx"

class Package {
public:
    Package();

    explicit Package(ElementID id) : id_(id) {
        assigned_ids_.insert(id_);
    }

    Package(Package&& package) : id_(package.id_) {}
    Package& operator=(Package&& package) noexcept;
    ~Package(); //

    ElementID get_id() const { return id_; }

private:
    ElementID id_;

    static std::set<ElementID> assigned_ids_;
    static std::set<ElementID> freed_ids_;
};