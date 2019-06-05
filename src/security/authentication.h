/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <grpcpp/security/server_credentials.h>
#include <grpcpp/security/auth_metadata_processor.h>

using grpc::ServerCredentials;
using grpc::SslServerCredentialsOptions;
using grpc::Status;

/*
 * Authenticate request with username/password comparaison
 * by using metadata fields.
 */
class UserPassAuthenticator final : public grpc::AuthMetadataProcessor {
  public:
    UserPassAuthenticator(std::string user, std::string pass)
      : username(user), password(pass) {}
    ~UserPassAuthenticator() {}

    Status Process(const InputMetadata& auth_metadata,
                   grpc::AuthContext* context,
                   OutputMetadata* consumed_auth_metadata,
                   OutputMetadata* response_metadata) override;

  private:
    std::string username, password;
};


/* Supported Authentication methods */
enum AuthType {
  INSECURE,       // No Username/password, no encryption
  USERPASS_TLS,   // Username/password, TLS encryption
  MUTUAL_TLS      // TLS authentication & encryption
};

class AuthBuilder {
  public:
    AuthBuilder() {}
    ~AuthBuilder() {}

    /* Return ServerCredentials according to enum EncryptType*/
    std::shared_ptr<ServerCredentials> build();

    /* TLS */
    AuthBuilder& setKeyPath(std::string keyPath);
    AuthBuilder& setCertPath(std::string certPath);
    AuthBuilder& setRootCertPath(std::string rootPath);

    /* Username/password */
    AuthBuilder& setUsername(std::string username);
    AuthBuilder& setPassword(std::string password);

    AuthBuilder& setInsecure(bool mode);

  private:
    std::string private_key_path, cert_path, root_cert_path; //SSL
    std::string username, password;
    bool insecure = false;
};

