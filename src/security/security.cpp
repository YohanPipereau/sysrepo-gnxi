/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <fstream>

#include <utils/log.h>

#include "security.h"

using std::string;
using std::shared_ptr;
using std::istreambuf_iterator;
using grpc::Status;
using grpc::StatusCode;

/* GetFileContent - Get an entier File content
 * @param Path to the file
 * @return String containing the entire File.
 */
string GetFileContent(string path)
{
  std::ifstream ifs(path);
  if (!ifs) {
    BOOST_LOG_TRIVIAL(fatal) << "File " << path << " not found";
    exit(1);
  }

  string content((istreambuf_iterator<char>(ifs)),
                 (istreambuf_iterator<char>()));

  ifs.close();

  return content;
}

/* SslCredentialsHelper -
 * @param ppath Private Key path
 * @param cpath certificates path
 * @return ServerCredentials for grpc service creation
 */
std::shared_ptr<ServerCredentials>
SslCredentialsHelper(string ppath, string cpath, string rpath)
{
  SslServerCredentialsOptions
        ssl_opts(GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY);

  SslServerCredentialsOptions::PemKeyCertPair pkcp = {
    GetFileContent(ppath),
    GetFileContent(cpath)
  };

  ssl_opts.pem_key_cert_pairs.push_back(pkcp);

  if (!rpath.empty()) {
    ssl_opts.pem_root_certs = GetFileContent(rpath);
  } else {
    ssl_opts.pem_root_certs = "";
  }

  return grpc::SslServerCredentials(ssl_opts);
}

/* Get Server Credentials according to Scurity Context policy */
std::shared_ptr<ServerCredentials> ServerSecurityContext::GetCredentials()
{
  std::shared_ptr<ServerCredentials> servCred;

  if (encType == SSL) {
    servCred = SslCredentialsHelper(private_key_path, cert_path, root_cert_path);
  } else if (encType == INSECURE) {
    servCred = grpc::InsecureServerCredentials();
  } else {
    BOOST_LOG_TRIVIAL(fatal) << "Unknown Encryption Type";
    exit(1);
  }

  if (authType == NOAUTH)
    return servCred;
  else if (authType == USERPASS && encType == SSL) {
      servCred->SetAuthMetadataProcessor(shared_ptr<UserPassProcessor>(proc));
      return servCred;
  } else if (authType == USERPASS && encType == INSECURE) {
    BOOST_LOG_TRIVIAL(fatal) << "Impossible to use user/pass auth with"
                             << " insecure connection";
    exit(1);
  } else {
    BOOST_LOG_TRIVIAL(fatal) << "Unknown Authentication Type";
    exit(1);
  }
}

/* Implement a MetadataProcessor for username/password authentication */
Status UserPassProcessor::Process(const InputMetadata& auth_metadata,
                                      grpc::AuthContext* context,
                                      OutputMetadata* consumed_auth_metadata,
                                      OutputMetadata* response_metadata)
{
  (void)context; (void)response_metadata; //Unused

  /* Look for username/password fields in Metadata sent by client */
  auto user_kv = auth_metadata.find("username");
  if (user_kv == auth_metadata.end()) {
    BOOST_LOG_TRIVIAL(error) << "No username field";
    return Status(StatusCode::UNAUTHENTICATED, "No username field");
  }
  auto pass_kv = auth_metadata.find("password");
  if (pass_kv == auth_metadata.end()) {
    BOOST_LOG_TRIVIAL(error) << "No password field";
    return Status(StatusCode::UNAUTHENTICATED, "No password field");
  }

  /* test if username and password are good */
  if ( password.compare(pass_kv->second.data()) != 0 ||
       username.compare(user_kv->second.data()) != 0 ) {
    BOOST_LOG_TRIVIAL(error) << "Invalid username/password";
    return Status(StatusCode::UNAUTHENTICATED, "Invalid username/password");
  }

  /* Remove username and password key-value from metadata */
  consumed_auth_metadata->insert(std::make_pair(
        string(user_kv->first.data(), user_kv->first.length()),
        string(user_kv->second.data(), user_kv->second.length())));
  consumed_auth_metadata->insert(std::make_pair(
        string(pass_kv->first.data(), pass_kv->first.length()),
        string(pass_kv->second.data(), pass_kv->second.length())));

  return Status::OK;
}
