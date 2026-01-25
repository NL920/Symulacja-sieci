#pragma once
#include <random>
#include <functional>

extern std::random_device rd;
extern std::mt19937 rng;

double probability_generator_1();
extern std::function<double()> probability_generator;

