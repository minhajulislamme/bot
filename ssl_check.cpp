#include <iostream>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

void print_ssl_error()
{
    unsigned long err;
    while ((err = ERR_get_error()) != 0)
    {
        char *str = ERR_error_string(err, NULL);
        std::cerr << "SSL Error: " << str << std::endl;
    }
}

bool test_ssl_connection(const std::string &hostname, int port)
{
    // Initialize all variables at the beginning of the function
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    BIO *bio = NULL;
    bool result = false;
    std::string conn_str = hostname + ":" + std::to_string(port);

    // Initialize SSL library
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    // Create new context
    const SSL_METHOD *method = TLS_client_method();
    ctx = SSL_CTX_new(method);
    if (!ctx)
    {
        std::cerr << "SSL_CTX_new failed" << std::endl;
        print_ssl_error();
        goto cleanup;
    }

    // Set default verify paths
    if (!SSL_CTX_set_default_verify_paths(ctx))
    {
        std::cerr << "SSL_CTX_set_default_verify_paths failed" << std::endl;
        print_ssl_error();
        goto cleanup;
    }

    // Create BIO
    bio = BIO_new_ssl_connect(ctx);
    if (!bio)
    {
        std::cerr << "BIO_new_ssl_connect failed" << std::endl;
        print_ssl_error();
        goto cleanup;
    }

    // Get SSL object
    BIO_get_ssl(bio, &ssl);
    if (!ssl)
    {
        std::cerr << "BIO_get_ssl failed" << std::endl;
        print_ssl_error();
        goto cleanup;
    }

    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    // Set connection string
    BIO_set_conn_hostname(bio, conn_str.c_str());

    // Connect
    if (BIO_do_connect(bio) <= 0)
    {
        std::cerr << "BIO_do_connect failed" << std::endl;
        print_ssl_error();
        goto cleanup;
    }

    // Verify
    if (BIO_do_handshake(bio) <= 0)
    {
        std::cerr << "BIO_do_handshake failed" << std::endl;
        print_ssl_error();
        goto cleanup;
    }

    std::cout << "SSL connection established successfully!" << std::endl;
    result = true;

cleanup:
    if (bio)
        BIO_free_all(bio);
    if (ctx)
        SSL_CTX_free(ctx);

    // Cleanup OpenSSL
    ERR_free_strings();
    EVP_cleanup();

    return result;
}

int main()
{
    std::cout << "Testing SSL connection to Binance WebSocket server..." << std::endl;

    bool success = test_ssl_connection("stream.binancefuture.com", 443);

    if (success)
    {
        std::cout << "SSL connection test passed!" << std::endl;
        return 0;
    }
    else
    {
        std::cerr << "SSL connection test failed!" << std::endl;
        return 1;
    }
}
