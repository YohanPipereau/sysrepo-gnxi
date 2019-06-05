/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <fstream>

#include <utils/log.h>

#include "authentication.h"

using namespace std;
using grpc::Status;
using grpc::StatusCode;

/* GetFileContent - Get an entier File content
 * @param Path to the file
 * @return String containing the entire File.
 */
static string GetFileContent(string path)
{
  ifstream ifs(path);
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
 * @param rpath root certificate path
 * @param client_cert boolean to activate/deactivate client certificate check
 * @return ServerCredentials for grpc service creation
 */
static shared_ptr<ServerCredentials>
SslCredentialsHelper(string ppath, string cpath, string rpath, bool client_cert)
{
  SslServerCredentialsOptions ssl_opts;

  if (client_cert)
    ssl_opts.client_certificate_request =
     GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY;
  else
    ssl_opts.client_certificate_request =
     GRPC_SSL_DONT_REQUEST_CLIENT_CERTIFICATE;

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
shared_ptr<ServerCredentials> AuthBuilder::build()
{
  shared_ptr<ServerCredentials> cred;

  // MUTUAL_TLS
  if (!private_key_path.empty() && !cert_path.empty()
      && username.empty() && password.empty()) {
    BOOST_LOG_TRIVIAL(info) << "Mutual TLS authentication";
    return SslCredentialsHelper(private_key_path, cert_path, root_cert_path, true);
  }

  // USERPASS_TLS
  if (!private_key_path.empty() && !cert_path.empty()
      && !username.empty() && !password.empty()) {
    BOOST_LOG_TRIVIAL(info) << "Username/Password over TLS authentication";
    cred = SslCredentialsHelper(private_key_path, cert_path, root_cert_path, false);
    cred->SetAuthMetadataProcessor(
        make_shared<UserPassAuthenticator>(username, password));
    return cred;
  }

  //INSECURE
  if (insecure) {
    BOOST_LOG_TRIVIAL(info) << "Insecure authentication";
    return grpc::InsecureServerCredentials();
  }

  /* impossible scenario */
  if (private_key_path.empty() && cert_path.empty()
      && !username.empty() && !password.empty())
    BOOST_LOG_TRIVIAL(fatal) << "Impossible to use user/pass auth with"
                             << " insecure connection";


  BOOST_LOG_TRIVIAL(fatal) << "Unsupported Authentication method";

  exit(1);
}

AuthBuilder& AuthBuilder::setKeyPath(string keyPath)
{
  private_key_path = keyPath;
  return *this;
}

AuthBuilder& AuthBuilder::setCertPath(string certPath)
{
  cert_path = certPath;
  return *this;
}

AuthBuilder& AuthBuilder::setRootCertPath(string rootpath)
{
  root_cert_path = rootpath;
  return *this;
}

AuthBuilder& AuthBuilder::setUsername(string user)
{
  username = user;
  return *this;
}

AuthBuilder& AuthBuilder::setPassword(string pass)
{
  password = pass;
  return *this;
}

AuthBuilder& AuthBuilder::setInsecure(bool mode)
{
  insecure = mode;
  return *this;
}

/* Implement a MetadataProcessor for username/password authentication */
Status UserPassAuthenticator::Process(const InputMetadata& auth_metadata,
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
  consumed_auth_metadata->insert(make_pair(
        string(user_kv->first.data(), user_kv->first.length()),
        string(user_kv->second.data(), user_kv->second.length())));
  consumed_auth_metadata->insert(make_pair(
        string(pass_kv->first.data(), pass_kv->first.length()),
        string(pass_kv->second.data(), pass_kv->second.length())));

  return Status::OK;
}
