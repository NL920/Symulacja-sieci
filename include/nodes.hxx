#pragma once

#include "types.hxx"
#include "storage_types.hxx"
#include "package.hxx"

// Miejsce klas pracownika, magazynu i rampy

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
    IPackageQueue queue_;


};

class Warehouse {
    public :
    Warehouse(WarehouseID const ID) : id_(ID) {}
    ~Warehouse()=default;

    private :
    const WarehouseID id_;
};