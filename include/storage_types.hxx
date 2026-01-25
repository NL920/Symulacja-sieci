#pragma once
#include <cstddef>
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

/*
IPackageQueue uzyte bedzie w Worker
- Umiesc polprodukt w konterze
- Przegladaj produkty
- Usun produkt
- Rozwija o dodanie typu kolejki
*/

class IPackageStockpile {
public:
    using const_iterator = std::list<Package>::const_iterator;
    // umiesc produkt
    virtual void push(Package&& moved) = 0;
    //iteratory
    virtual const_iterator begin() const = 0;
    virtual const_iterator cbegin() const = 0;
    virtual const_iterator end() const = 0;
    virtual const_iterator cend() const = 0;
    // metoda zwracajaca ilosc w magazynie
    virtual size_t size() const = 0;
    // metoda sprawdzajaca czy w magazynie cos jest
    virtual bool empty() const = 0;
    virtual ~IPackageStockpile() = default;
};

class IPackageQueue : public IPackageStockpile {
public:
    ~IPackageQueue() override = default;
    virtual Package pop() = 0; // Wyciaganie produktu ze skladzika
    virtual PackageQueueType get_queue_type() const = 0;
};

class PackageQueue : public IPackageQueue {
    public:
    PackageQueue() = delete;
    explicit PackageQueue(PackageQueueType const type) : type_(type), inventory_() {}
    void push(Package&& moved) override {inventory_.emplace_back(std::move(moved));}

    const_iterator begin() const override {return inventory_.cbegin();}
    const_iterator cbegin() const override {return inventory_.cbegin();}
    const_iterator end() const override {return inventory_.cend();}
    const_iterator cend() const override {return inventory_.cend();}

    size_t size() const override {return inventory_.size();}
    bool empty() const override {return inventory_.empty();}
    Package pop() override;
    PackageQueueType get_queue_type() const override {return type_;}

    ~PackageQueue() override = default;

    private:
    PackageQueueType type_;
    std::list<Package> inventory_;
};


/*
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
*/



