/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <grpcpp/security/server_credentials.h>
#include <grpcpp/security/auth_metadata_processor.h>

using grpc::ServerCredentials;
using grpc::SslServerCredentialsOptions;
using grpc::Status;

class UserPassProcessor final : public grpc::AuthMetadataProcessor {
  public:
    Status Process(const InputMetadata& auth_metadata,
                   grpc::AuthContext* context,
                   OutputMetadata* consumed_auth_metadata,
                   OutputMetadata* response_metadata) override;

  private:
    std::string username, password;

  friend class ServerSecurityContext;
};

/* Supported Authentication methods */
enum AuthType {
  NOAUTH,
  USERPASS
};

/* Supported Encryption methods */
enum EncryptType {
  SSL,
  INSECURE
};

/*
 * Represents server security context. Carries:
 * - Encryption informations : Either Tls or Insecure
 * - Authentication informations: Based on Metadata Processor or Interceptor
 */
class ServerSecurityContext {
  public:
    ServerSecurityContext() : encType(SSL), authType(NOAUTH)
      {proc = new UserPassProcessor();};
    ~ServerSecurityContext() {delete proc;};

    /* Return ServerCredentials according to enum EncryptType*/
    std::shared_ptr<ServerCredentials> GetCredentials();
    /* Set/Get Paths for PEM keys and certs */
    std::string GetKeyPath() {return private_key_path;};
    std::string GetCertsPath() {return chain_certs_path;};
    void SetKeyPath(std::string keyPath) {private_key_path = keyPath;};
    void SetCertsPath(std::string certsPath) {chain_certs_path = certsPath;};
    /* Authentication Processor to parse message metadata */
    void SetUsername(std::string user) {proc->username = user;};
    void SetPassword(std::string pass) {proc->password = pass;};
    std::string GetUsername() {return proc->username;};
    std::string GetPassword() {return proc->password;};
    /* Set/Get Encryption Type of this security context */
    void SetEncryptType(enum EncryptType type) {encType = type;};
    enum EncryptType GetEncryptType() {return encType;};
    /* Set/Get Authentication Type of this security context */
    void SetAuthType(enum AuthType type) {authType = type;};
    enum AuthType GetAuthType() {return authType;};

    friend class UserPassProcessor;

  private:
    enum EncryptType encType;
    std::string private_key_path, chain_certs_path; //SSL

    enum AuthType authType;
    UserPassProcessor *proc;
};
