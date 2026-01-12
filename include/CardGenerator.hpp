#include <string>
#include <openssl/rand.h>
#include <unordered_set>
#include<vector>

class CardGenerator {

public:
    static std::string generate(const std::string& bin_prefix = "5");
    static bool validate(const std::string& number);
    static void pregenerate(size_t count);
    
private:
    static char calculateLuhn(const std::string& number);
    static std::string generateBIN(unsigned length);
    static std::unordered_set<std::string> issued_numbers_;
};