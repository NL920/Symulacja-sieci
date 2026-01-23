#include "storage_types.hxx"
#include <stdexcept>


Package IPackageQueue::getItem(){
    if (inventory_.empty()){
        throw std::out_of_range("IPackageQueue::getItem Próba pobrania z pustego inventory_");
    }
    if(type_ == PackageQueueType::FIFO){ // Kolejka
        Package product = inventory_.front();
        inventory_.erase(inventory_.begin()); // iterator usuwa 1 element
        return product;
    }

    else if(type_ == PackageQueueType::LIFO){ // Stos
        Package product = inventory_.back();
        inventory_.pop_back(); // usuwa ostatni element
        return product;
    }

    throw std::runtime_error("IPackageQueue::getItem błąd typu type_");
}