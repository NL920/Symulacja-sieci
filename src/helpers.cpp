#include "helpers.hxx"

std::random_device rd;
std::mt19937 rng(rd());

double probability_generator_1() {
    return std::generate_canonical<double, 10>(rng);
}

std::function<double()> probability_generator = probability_generator_1;
