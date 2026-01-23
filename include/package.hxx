// Klasa odpowiedzialna za polprodukt
#pragma once
#include <set>
#include "types.hxx"

class Package {
public:
    Package();

    Package(const Package&) = default;
    Package(Package&&) = default;

    ~Package(); //

    ElementID get_id() const { return id_; }

private:
    const ElementID id_;

    static std::set<ElementID> assigned_ids_;
    static std::set<ElementID> freed_ids_;
};