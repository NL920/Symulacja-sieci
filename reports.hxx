#pragma once
#ifndef REPORTS_HXX
#define REPORTS_HXX
#include <iostream>
#include <optional>
#include "factory.hxx"

void generate_structure_report(const Factory& f, std:: ostream& os);
void generate_simulation_turn_report(const Factory& f, std::ostream& os, Time t);

#endif