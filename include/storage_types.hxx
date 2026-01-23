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
class PackageStockpile {
    public:
    PackageStockpile() = default;
    ~PackageStockpile() = default;

    void addPackage(const Package& product){inventory_.emplace_back(product);}
    unsigned int inventorySize() const {return inventory_.size();} // getter rozmiaru
    // W zadaniu znajduje sie wymog const i nie const iteratorow po zawartosci, narazie pomijam bo niepotrzebne

    protected:
    std::list<Package> inventory_; // Domyslnie puste
};

/*
IPackageQueue uzyte bedzie w Worker
- Umiesc polprodukt w konterze
- Przegladaj produkty
- Usun produkt
- Rozwija o dodanie typu kolejki
*/
class PackageQueue : public PackageStockpile {
    public:
    PackageQueue(PackageQueueType const type) : type_(type){}
    ~PackageQueue() = default;
    
    // getter i setter (raczej adder xd) dziedziczone z IPackageStockpile
    Package getItem(); // Usuwa przedmiot z inventory_ zgodnie z type_, zwraca kopie usunietego półproduktu 

    private:
    PackageQueueType type_;
};