#include "storage_types.hxx"
#include <stdexcept>


Package PackageQueue::pop(){
    if (inventory_.empty()){
        throw std::out_of_range("IPackageQueue::getItem Próba pobrania z pustego inventory_");
    }
    if(type_ == PackageQueueType::FIFO){ // Kolejka
        Package product = std::move(inventory_.front()); // Pobieramy pierwszy
        inventory_.pop_front();                         // Usuwamy pierwszy
        return product;
    }

    else if(type_ == PackageQueueType::LIFO){ // Stos
        Package product = std::move(inventory_.back());  // Pobieramy ostatni
        inventory_.pop_back();                          // Usuwamy ostatni
        return product;
    }

    throw std::runtime_error("IPackageQueue::getItem błąd typu type_");
}