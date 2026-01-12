#ifndef SECURITYHELPER_HPP
#define SECURITYHELPER_HPP
#include<iostream>
#include<string>
#include<vector>
#include<iomanip>



/**
 * @class SecurityHelper
 * @brief Provides static utility functions for encryption, decryption, hashing, key management, and validation.
 * 
 * This class offers methods to encrypt/decrypt data, hash PINs with salts, generate cryptographic keys,
 * validate passwords, rotate encryption keys, and generate session tokens with expiration.
 */
class SecurityHelper{
        public:

            


            /**
             * @struct EncryptedData
             * @brief Container for encrypted data components.
             */
            struct EncryptedData 
            {
                std::string ciphertext;
                std::string iv;
                std::string key;
            };

            //------------------- Encryption/Decryption -------------------

            /**
             * @brief Encrypts the given plaintext string.
             * @param plaintext The input string to encrypt.
             * @return An EncryptedData struct containing ciphertext, iv, and key.
             */
            static EncryptedData encrypt(const std::string& plaintext);

            /**
             * @brief Decrypts the given encrypted data.
             * @param data The EncryptedData containing ciphertext, iv, and key.
             * @return The decrypted plaintext string.
             */
            static std::string decrypt(const EncryptedData& data);
            
            //---------------- Hashing ---------------- 


            /**
             * @brief Hashes a PIN using a salt.
             * @param pin The PIN string to hash.
             * @param salt The salt to use in hashing.
             * @return The resulting hashed string.
             */
            static std::string hashPin(const std::string& pin, const std::string& salt);
            
            // ---------------- Key Management ----------------

            /**
             * @brief Generates a cryptographic key into the provided buffer.
             * @param buffer Pointer to the buffer to store the key.
             * @param length Length of the key to generate.
             */
            static void generateKey(unsigned char* buffer, int length);

            /**
             * @brief Generates a random salt string.
             * @param length The length of the salt to generate.
             * @return A random salt string.
             */
            static std::string generateRandomSalt(size_t length = 32);
            
            // ---------------- Validation ----------------

            /**
             * @brief Validates the strength of a password.
             * @param password The password string to validate.
             * @return True if the password meets strength criteria, false otherwise.
             */
            static bool validatePassword(const std::string& password);


            /**
             * @struct RotatedKeys
             * @brief Contains new encrypted data and keys after key rotation.
             */
            struct RotatedKeys {
                EncryptedData new_data; ///< New encrypted data after rotation.
                std::string new_key;  ///< New encryption key.
                std::string new_iv; ///< New initialization vector.
            };
                
            /**
             * @brief Rotates encryption keys for given encrypted data.
             * @param old_data The old encrypted data.
             * @param old_key The old encryption key.
             * @param old_iv The old initialization vector.
             * @return A RotatedKeys struct with new encrypted data and keys.
             */
            static RotatedKeys rotateKeys(const EncryptedData& old_data,
                                          const std::string& old_key,
                                          const std::string& old_iv);
                                          struct SessionToken
                                         {
                                            std::string token; //< The session token string.
                                            std::time_t expiration;  ///< Expiration time (timestamp).
                                         };
                                        

            /**
             * @brief Generates a session token valid for a specified number of seconds.
             * @param validity_seconds The number of seconds the token should be valid (default 3600).
             * @return A SessionToken struct containing the token and expiration time.
             */
            static SessionToken generateSessionToken(int validity_seconds = 3600);

    private:
            /**
             * @brief Securely cleans memory by overwriting data.
             * @param data Pointer to the data to clean.
             * @param length Length of data in bytes.
             */
            static void secureClean(void* data, size_t length);
};

#endif