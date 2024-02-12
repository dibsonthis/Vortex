#include <iostream>
#include <string>
#include "include/openssl/evp.h"
#include "include/openssl/rand.h"
#include "include/openssl/rsa.h"
#include "include/openssl/pem.h"
#include "include/openssl/err.h"
#include "include/Vortex.hpp"

extern "C" Value evp_generate_key(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        return error_object("Function 'generate_key' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value size = args[0];

    if (!size.is_number())
    {
        return error_object("Parameter 'size' must be a number");
    }

    int keySize = size.get_number();

    std::string _key;
    _key.resize(keySize);
    RAND_bytes(reinterpret_cast<unsigned char *>(&_key[0]), keySize);

    Value key = string_val(_key);

    return key;
}

extern "C" Value evp_encrypt(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        return error_object("Function 'encrypt' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value str = args[0];
    Value key_ = args[1];

    if (!str.is_string())
    {
        return error_object("Parameter 'str' must be a string");
    }

    if (!key_.is_string())
    {
        return error_object("Parameter 'key' must be a string");
    }

    std::string &key = key_.get_string();
    std::string &plainText = str.get_string();

    if (key.length() < 32)
    {
        std::cout << "Crypto Error: Key length must be at least 32" << std::endl;
        return string_val("");
    }

    // Generate IV
    unsigned char iv[EVP_MAX_IV_LENGTH];
    RAND_bytes(iv, EVP_MAX_IV_LENGTH);

    // Set up OpenSSL cipher context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
    {
        std::cout << "Crypto Error: Error creating cipher context" << std::endl;
        return string_val("");
    }

    // Initialize encryption operation
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char *>(key.c_str()), iv) != 1)
    {
        std::cout << "Crypto Error: Error initializing encryption operation" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return string_val("");
    }

    // Perform encryption
    int cipherTextLen = plainText.size() + EVP_MAX_BLOCK_LENGTH; // Ensure enough space for padding
    unsigned char *cipherText = new unsigned char[cipherTextLen];
    int actualCipherLen = 0;
    if (EVP_EncryptUpdate(ctx, cipherText, &actualCipherLen, reinterpret_cast<const unsigned char *>(plainText.c_str()), plainText.size()) != 1)
    {
        std::cout << "Crypto Error: Error performing encryption" << std::endl;
        delete[] cipherText;
        EVP_CIPHER_CTX_free(ctx);
        return string_val("");
    }

    int finalLen = 0;
    if (EVP_EncryptFinal_ex(ctx, cipherText + actualCipherLen, &finalLen) != 1)
    {
        std::cout << "Crypto Error: Error finalizing encryption" << std::endl;
        delete[] cipherText;
        EVP_CIPHER_CTX_free(ctx);
        return string_val("");
    }

    actualCipherLen += finalLen;

    // Concatenate IV and ciphertext
    std::string encryptedText(reinterpret_cast<char *>(iv), EVP_MAX_IV_LENGTH);
    encryptedText.append(reinterpret_cast<char *>(cipherText), actualCipherLen);

    // Clean up
    delete[] cipherText;
    EVP_CIPHER_CTX_free(ctx);

    return string_val(encryptedText);
}

extern "C" Value evp_decrypt(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        return error_object("Function 'decrypt' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value str = args[0];
    Value key_ = args[1];

    if (!str.is_string())
    {
        return error_object("Parameter 'str' must be a string");
    }

    if (!key_.is_string())
    {
        return error_object("Parameter 'key' must be a string");
    }

    std::string &key = key_.get_string();
    std::string &encryptedText = str.get_string();

    if (key.length() < 32)
    {
        std::cout << "Crypto Error: Key length must be at least 32" << std::endl;
        return string_val("");
    }

    // Extract IV from the encrypted text
    unsigned char iv[EVP_MAX_IV_LENGTH];
    std::memcpy(iv, encryptedText.c_str(), EVP_MAX_IV_LENGTH);

    // Set up OpenSSL cipher context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
    {
        std::cout << "Crypto Error: Error creating cipher context" << std::endl;
        return string_val("");
    }

    // Initialize decryption operation
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char *>(key.c_str()), iv) != 1)
    {
        std::cout << "Crypto Error: Error initializing decryption operation" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return string_val("");
    }

    // Allocate memory for plaintext
    int maxPlainTextLen = encryptedText.size() - EVP_MAX_IV_LENGTH; // Max possible plaintext size

    if (maxPlainTextLen < 0)
    {
        std::cout << "Crypto Error: Error decoding encrypted text" << std::endl;
        return string_val("");
    }

    unsigned char *plainText = new unsigned char[maxPlainTextLen];

    int actualPlainLen = 0;
    // Perform decryption
    if (EVP_DecryptUpdate(ctx, plainText, &actualPlainLen, reinterpret_cast<const unsigned char *>(encryptedText.c_str()) + EVP_MAX_IV_LENGTH, encryptedText.size() - EVP_MAX_IV_LENGTH) != 1)
    {
        std::cout << "Crypto Error: Error performing decryption" << std::endl;
        delete[] plainText;
        EVP_CIPHER_CTX_free(ctx);
        return string_val("");
    }

    int finalLen = 0;
    // Finalize decryption
    if (EVP_DecryptFinal_ex(ctx, plainText + actualPlainLen, &finalLen) != 1)
    {
        std::cout << "Crypto Error: Error finalizing decryption" << std::endl;
        delete[] plainText;
        EVP_CIPHER_CTX_free(ctx);
        return string_val("");
    }

    actualPlainLen += finalLen;

    // Construct the plaintext string
    std::string decryptedText(reinterpret_cast<char *>(plainText), actualPlainLen);

    // Clean up
    delete[] plainText;
    EVP_CIPHER_CTX_free(ctx);

    return string_val(decryptedText);
}

extern "C" Value rsa_generate_pair(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        return error_object("Function 'generate_pair' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value size = args[0];

    if (!size.is_number())
    {
        return error_object("Parameter 'size' must be a number");
    }

    int modulusSize = size.get_number();

    Value pair_obj = object_val();
    pair_obj.get_object()->keys = {"private", "public"};
    pair_obj.get_object()->values["private"] = string_val("");
    pair_obj.get_object()->values["public"] = string_val("");

    if (modulusSize < 2048)
    {
        std::cerr << "Crypto Error: Modulus size must be at least 2048" << std::endl;
    }

    RSA *rsaKeyPair = RSA_generate_key(modulusSize, RSA_F4, nullptr, nullptr);
    if (!rsaKeyPair)
    {
        std::cerr << "Error generating RSA key pair" << std::endl;
        return pair_obj;
    }

    // Convert private key to PEM-encoded string
    BIO *bioPrivateKey = BIO_new(BIO_s_mem());
    if (!bioPrivateKey)
    {
        std::cerr << "Error creating BIO for private key" << std::endl;
        RSA_free(rsaKeyPair);
        return pair_obj;
    }
    PEM_write_bio_RSAPrivateKey(bioPrivateKey, rsaKeyPair, nullptr, nullptr, 0, nullptr, nullptr);
    char *privateKeyBuffer;
    long privateKeyLength = BIO_get_mem_data(bioPrivateKey, &privateKeyBuffer);
    std::string privateKeyString(privateKeyBuffer, privateKeyLength);
    BIO_free(bioPrivateKey);

    // Convert public key to PEM-encoded string
    BIO *bioPublicKey = BIO_new(BIO_s_mem());
    if (!bioPublicKey)
    {
        std::cerr << "Error creating BIO for public key" << std::endl;
        RSA_free(rsaKeyPair);
        return pair_obj;
    }
    PEM_write_bio_RSAPublicKey(bioPublicKey, rsaKeyPair);
    char *publicKeyBuffer;
    long publicKeyLength = BIO_get_mem_data(bioPublicKey, &publicKeyBuffer);
    std::string publicKeyString(publicKeyBuffer, publicKeyLength);
    BIO_free(bioPublicKey);

    // Free RSA key pair
    RSA_free(rsaKeyPair);

    pair_obj.get_object()->values["private"] = string_val(privateKeyString);
    pair_obj.get_object()->values["public"] = string_val(publicKeyString);

    return pair_obj;
}

extern "C" Value rsa_encrypt(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        return error_object("Function 'encrypt' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value str = args[0];
    Value key_ = args[1];

    if (!str.is_string())
    {
        return error_object("Parameter 'str' must be a string");
    }

    if (!key_.is_string())
    {
        return error_object("Parameter 'key' must be a string");
    }

    std::string &keyString = key_.get_string();
    std::string &plainText = str.get_string();

    if (keyString.length() * 8 < 2048)
    {
        std::cout << "Crypto Error: Key length must be at least 2048" << std::endl;
        return string_val("");
    }

    RSA *rsa = nullptr;
    BIO *bio = BIO_new_mem_buf(keyString.c_str(), keyString.length());
    if (!bio)
    {
        std::cerr << "Crypto Error: Error creating BIO" << std::endl;
        return string_val("");
    }

    rsa = PEM_read_bio_RSAPublicKey(bio, nullptr, nullptr, nullptr);

    if (!rsa)
    {
        std::cerr << "Crypto Error: Error loading RSA key from PEM string" << std::endl;
        ERR_print_errors_fp(stderr);
        BIO_free(bio);
        return string_val("");
    }

    BIO_free(bio);

    int rsaLen = RSA_size(rsa);
    unsigned char *cipherText = new unsigned char[rsaLen];
    int cipherLen = RSA_public_encrypt(plainText.size(), reinterpret_cast<const unsigned char *>(plainText.c_str()), cipherText, rsa, RSA_PKCS1_PADDING);
    if (cipherLen == -1)
    {
        std::cerr << "Crypto Error: Error encrypting data" << std::endl;
        delete[] cipherText;
        return string_val("");
    }
    std::string encryptedText(reinterpret_cast<char *>(cipherText), cipherLen);
    delete[] cipherText;

    return string_val(encryptedText);
}

extern "C" Value rsa_decrypt(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        return error_object("Function 'decrypt' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value str = args[0];
    Value key_ = args[1];

    if (!str.is_string())
    {
        return error_object("Parameter 'str' must be a string");
    }

    if (!key_.is_string())
    {
        return error_object("Parameter 'key' must be a string");
    }

    std::string &keyString = key_.get_string();
    std::string &encryptedText = str.get_string();

    if (keyString.length() * 8 < 2048)
    {
        std::cout << "Crypto Error: Key length must be at least 2048" << std::endl;
        return string_val("");
    }

    RSA *rsa = nullptr;
    BIO *bio = BIO_new_mem_buf(keyString.c_str(), keyString.length());
    if (!bio)
    {
        std::cerr << "Crypto Error: Error creating BIO" << std::endl;
        return string_val("");
    }

    rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);

    if (!rsa)
    {
        std::cerr << "Crypto Error: Error loading RSA key from PEM string" << std::endl;
        ERR_print_errors_fp(stderr);
        BIO_free(bio);
        return string_val("");
    }

    BIO_free(bio);

    int rsaLen = RSA_size(rsa);
    unsigned char *decryptedText = new unsigned char[rsaLen];
    int plainLen = RSA_private_decrypt(encryptedText.size(), reinterpret_cast<const unsigned char *>(encryptedText.c_str()), decryptedText, rsa, RSA_PKCS1_PADDING);

    if (plainLen == -1)
    {
        std::cerr << "Error decrypting data" << std::endl;
        delete[] decryptedText;
        return string_val("");
    }
    std::string plainText(reinterpret_cast<char *>(decryptedText), plainLen);
    delete[] decryptedText;

    return string_val(plainText);
}