/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <fstream>

#include "security.h"

using std::string;
using std::shared_ptr;

/* GetFileContent - Get an entier File content
 * @param Path to the file
 * @return String containing the entire File.
 */
std::string GetFileContent(std::string path)
{
  std::ifstream ifs(path);
  if (!ifs) {
    std::cerr << "File" << path << " not found" << std::endl;
    exit(1);
  }

  std::string content((std::istreambuf_iterator<char>(ifs)),
      (std::istreambuf_iterator<char>()));

  ifs.close();

  return content;
}

/* SslCredentialsHelper -
 * @param ppath Private Key path
 * @param cpath certificates path
 * @return
 */
std::shared_ptr<ServerCredentials>
SslCredentialsHelper(string ppath, string cpath)
{
  SslServerCredentialsOptions
        ssl_opts(GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY);

  SslServerCredentialsOptions::PemKeyCertPair pkcp = {
    GetFileContent(ppath),
    GetFileContent(cpath)
  };

  ssl_opts.pem_root_certs = "";
  ssl_opts.pem_key_cert_pairs.push_back(pkcp);

  return grpc::SslServerCredentials(ssl_opts);
}

/* Get Server Credentials according to Scurity Context policy */
std::shared_ptr<ServerCredentials> ServerSecurityContext::GetCredentials()
{
  std::shared_ptr<ServerCredentials> servCred;

  if (encType == SSL) {
      servCred = SslCredentialsHelper(private_key_path, chain_certs_path);
    } else if (encType == INSECURE) {
        servCred = grpc::InsecureServerCredentials();
    } else {
      std::cerr << "Unknown Encryption Type" << std::endl;
      exit(1);
    }

  if (authType == NOAUTH)
    return servCred;
  else if (authType == USERPASS && encType == SSL) {
      servCred->SetAuthMetadataProcessor(shared_ptr<UserPassProcessor>(proc));
      return servCred;
  } else if (authType == USERPASS && encType == INSECURE) {
    std::cerr << "Impossible to use user/pass auth with insecure connection"
      << std::endl;
    exit(1);
  } else {
    std::cerr << "Unknown Authentication Type" << std::endl;
    exit(1);
  }
}

/* Implement a MetadataProcessor for username/password authentication */
Status UserPassProcessor::Process(const InputMetadata& auth_metadata,
                                      grpc::AuthContext* context,
                                      OutputMetadata* consumed_auth_metadata,
                                      OutputMetadata* response_metadata)
{
  /* Look for username/password fields in Metadata sent by client */
  auto user_kv = auth_metadata.find("username");
  if (user_kv == auth_metadata.end()) {
    std::cerr << "No username field" << std::endl;
    return grpc::Status(grpc::StatusCode::UNAUTHENTICATED,
                        "No username field");
  }
  auto pass_kv = auth_metadata.find("password");
  if (pass_kv == auth_metadata.end()) {
    std::cerr << "No password field" << std::endl;
    return grpc::Status(grpc::StatusCode::UNAUTHENTICATED,
                        "No password field");
  }

  /* test if username and password are good */
  if (password != pass_kv->second.data() ||
      username != user_kv->second.data()) {
    std::cerr << "Invalid username/password" << std::endl;
    return grpc::Status(grpc::StatusCode::UNAUTHENTICATED,
                        "Invalid username/password");
  }

  /* Remove username and password key-value from metadata */
  consumed_auth_metadata->insert(std::make_pair(
        string(user_kv->first.data(), user_kv->first.length()),
        string(user_kv->second.data(), user_kv->second.length())));
  consumed_auth_metadata->insert(std::make_pair(
        string(pass_kv->first.data(), pass_kv->first.length()),
        string(pass_kv->second.data(), pass_kv->second.length())));

  return grpc::Status::OK;
}
