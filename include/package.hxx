// Klasa odpowiedzialna za polprodukt
#pragma once
#include "types.hxx"

class Package {
    public:
    Package(ElementID ID) : id_(ID){}
    ~Package(){} // Metoda zwalniania ID w przyszlosci 

    private:
    const ElementID id_; // ID produktu ma byc niezmienialne
};