#include "CardGenerator.hpp"
#include <stdexcept>
#include <algorithm>
#include<vector>


std::unordered_set<std::string> CardGenerator::issued_numbers_;

std::string CardGenerator::generate(const std::string& bin_prefix) {
    // Validate BIN format
    if (!std::all_of(bin_prefix.begin(), bin_prefix.end(), ::isdigit)) {
        throw std::invalid_argument("BIN must contain only digits");
    }

    std::string number = bin_prefix;
    
    // Replace VLA with dynamic allocation
    const int remaining_length = 16 - bin_prefix.length() - 1; // Total 16 digits
    std::vector<unsigned char> rand_bytes(remaining_length);
    
    if (RAND_bytes(rand_bytes.data(), rand_bytes.size()) != 1) {
        throw std::runtime_error("Secure random generation failed");
    }

    for (size_t i = 0; i < rand_bytes.size(); ++i) {
        number += '0' + (rand_bytes[i] % 10);
    }

    // Calculate and append Luhn check digit
    number += calculateLuhn(number);
    issued_numbers_.insert(number);
    
    return number;
}


char CardGenerator::calculateLuhn(const std::string& number) {
    int sum = 0;
    bool alternate = false;
    
    for (auto it = number.rbegin(); it != number.rend(); ++it) {
        int n = *it - '0';
        if (alternate) {
            n *= 2;
            if (n > 9) n = (n / 10) + (n % 10);
        }
        sum += n;
        alternate = !alternate;
    }
    
    return '0' + ((10 - (sum % 10))) % 10;
}

std::string CardGenerator::generateBIN(unsigned length) {
    unsigned char bin_bytes[8];
    if (RAND_bytes(bin_bytes, length)) {
        std::string bin;
        for (unsigned i = 0; i < length; ++i) {
            bin += '0' + (bin_bytes[i] % 10);
        }
        return bin;
    }
    throw std::runtime_error("BIN generation failed");
}

bool CardGenerator::validate(const std::string& number) {
    if (number.length() < 13 || number.length() > 19) return false;
    
    std::string base = number.substr(0, number.length()-1);
    char actual_check = number.back();
    char calculated_check = calculateLuhn(base);
    
    return (actual_check == calculated_check) && 
           (issued_numbers_.count(number));
}

// CardGenerator.cpp
void CardGenerator::pregenerate(size_t count) {
    for (size_t i = 0; i < count; ++i) {
        std::string number = generate("5");  // Using default prefix
        // Number is automatically added to issued_numbers_
    }
}
 