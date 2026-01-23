#pragma once

#include <list>
#include "package.hxx"

enum class PackageQueueType{
    FIFO,
    LIFO
};

/*
IPackageStockpile uzyte bedzie w Warehouse
- Umiesc polprodukt w konterze
- Przegladaj produkty
*/
class IPackageStockpile { 
    public:
    IPackageStockpile(){}
    ~IPackageStockpile() = default;

    void addPackage(const Package& product){inventory_.push_back(product);} 
    unsigned int inventorySize() const {return inventory_.size();} // getter rozmiaru

    private:
    std::list<Package> inventory_; // Domyslnie puste
};

/*
IPackageQueue uzyte bedzie w Worker
- Umiesc polprodukt w konterze
- Przegladaj produkty
- Usun produkt
- Rozwija o dodanie typu kolejki
*/
class IPackageQueue : public IPackageStockpile { 
    public:
    IPackageQueue(PackageQueueType const type) : type_(type){}
    ~IPackageQueue() = default;
    
    // getter i setter (raczej adder xd) dziedziczone z IPackageStockpile
    Package getItem(); // Usuwa przedmiot z inventory_ zgodnie z type_, zwraca kopie usunietego półproduktu 

    private:
    PackageQueueType type_;
    std::list<Package> inventory_;

};