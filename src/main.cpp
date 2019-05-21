/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <iostream>
#include <memory>
#include <chrono>
#include <getopt.h>

#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include "security/security.h"
#include "utils/log.h"
#include "server.h"

using namespace std;

void RunServer(ServerSecurityContext *cxt)
{
  std::string server_address("0.0.0.0:50051");
  GNMIServer service("sysepo-gnmi");
  ServerBuilder builder;

  builder.AddListeningPort(server_address, cxt->GetCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Using grpc " << grpc::Version() << std::endl;
  std::cout << "Server listening on " << server_address << std::endl;

  server->Wait();
}

static void show_usage(std::string name)
{
  std::cerr << "Usage: " << name << " <option(s)>\n"
    << "Options:\n"
    << "\t-h,--help\t\t\tShow this help message\n"
    << "\t-u,--username USERNAME\t\tDefine connection username\n"
    << "\t-p,--password PASSWORD\t\tDefine connection password\n"
    << "\t-f,--force-insecure\t\tNo TLS connection, no password authentication\n"
    << "\t-k,--private-key PRIVATE_KEY\tpath to server PEM private key\n"
    << "\t-c,--cert-chain CERT_CHAIN\tpath to server PEM certificate chain\n"
    << "\t-l,--log-level LOG_LEVEL\tLog level\n"
    << std::endl;
}

int main (int argc, char* argv[]) {
  int c;
  extern char *optarg;
  int option_index = 0;
  std::string username, password;
  ServerSecurityContext *cxt = new ServerSecurityContext();

  static struct option long_options[] =
  {
    {"help", no_argument, 0, 'h'},
    {"log-level", required_argument, 0, 'l'}, //log level
    {"username", required_argument, 0, 'u'},
    {"password", required_argument, 0, 'p'},
    {"private-key", required_argument, 0, 'k'}, //private key
    {"cert-chain", required_argument, 0, 'c'}, //certificate chain
    {"force-insecure", no_argument, 0, 'f'}, //insecure mode
    {0, 0, 0, 0}
  };

  /*
   * An option character followed by ('') indicates no argument
   * An option character followed by (‘:’) indicates a required argument.
   * An option character is followed by (‘::’) indicates an optional argument.
   * Here: no argument (h,f) ; mandatory arguments (p,u,l)
   */
  while ((c = getopt_long(argc, argv, "hfl:p:u:c:k:", long_options, &option_index))
         != -1) {
    switch (c)
    {
      case 'h':
        show_usage(argv[0]);
        exit(0);
        break;
      case 'u':
        if (optarg) {
          cxt->SetAuthType(USERPASS);
          cxt->SetUsername(string(optarg));
        } else {
          std::cerr << "Please specify a string with username option\n"
            << "Ex: -u USERNAME" << std::endl;
          exit(1);
        }
        break;
      case 'p':
        if (optarg) {
          cxt->SetAuthType(USERPASS);
          cxt->SetPassword(string(optarg));
        } else {
          std::cerr << "Please specify a string with password option\n"
            << "Ex: -p PASSWORD" << std::endl;
          exit(1);
        }
        break;
      case 'k':
        if (optarg) {
          cxt->SetKeyPath(string(optarg));
        } else {
          std::cerr << "Please specify a string with private key path\n"
            << "Ex: --private-key KEY_PATH" << std::endl;
          exit(1);
        }
        break;
      case 'c':
        if (optarg) {
          cxt->SetCertsPath(string(optarg));
        } else {
          std::cerr << "Please specify a string with chain certs path\n"
            << "Ex: --cert-chain CERTS_PATH" << std::endl;
          exit(1);
        }
        break;
      case 'l':
        if (optarg) {
          Log(atoi(optarg));
        } else {
          Log();
        }
        break;
      case 'f':
        cxt->SetEncryptType(INSECURE);
        break;
      case '?':
        show_usage(argv[0]);
        exit(1);
      default: /* You won't get there */
        exit(1);
    }
  }

  if (cxt->GetEncryptType() == EncryptType::SSL) {
    if (cxt->GetKeyPath().empty() || cxt->GetCertsPath().empty()) {
      std::cerr << "Both private key and certificate required" << std::endl;
      exit(1);
    }
    std::cout << "Initiate a TLS connection" << std::endl;
  }

  if (cxt->GetAuthType() == AuthType::USERPASS) {
    if (cxt->GetUsername().empty() || cxt->GetPassword().empty()) {
      std::cerr << "Both username and password required" << std::endl;
      exit(1);
    }
  }

  RunServer(cxt);

  return 0;
}
