/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <iostream>
#include <memory>
#include <chrono>
#include <getopt.h>

#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include "gnmi/gnmi.h"
#include <security/security.h>
#include <utils/log.h>

using namespace std;

void RunServer(ServerSecurityContext *cxt, string bind_addr)
{
  ServerBuilder builder;
  GNMIService gnmi("gnmi"); //gNMI Service

  builder.AddListeningPort(bind_addr, cxt->GetCredentials());
  builder.RegisterService(&gnmi);
  unique_ptr<Server> server(builder.BuildAndStart());
  cout << "Using grpc " << grpc::Version() << endl;

  if (bind_addr.find(":") == string::npos) {
    cout << "Server listening on " << bind_addr << ":443" << endl;
  } else {
    cout << "Server listening on " << bind_addr << endl;
  }

  server->Wait();
}

static void show_usage(string name)
{
  cerr << "Usage: " << name << " <option(s)>\n"
    << "Options:\n"
    << "\t-h,--help\t\t\tShow this help message\n"
    << "\t-u,--username USERNAME\t\tDefine connection username\n"
    << "\t-p,--password PASSWORD\t\tDefine connection password\n"
    << "\t-f,--force-insecure\t\tNo TLS connection, no password authentication\n"
    << "\t-k,--private-key PRIVATE_KEY\tpath to server PEM private key\n"
    << "\t-c,--cert-chain CERT_CHAIN\tpath to server PEM certificate chain\n"
    << "\t-l,--log-level LOG_LEVEL\tLog level\n"
    << "\t\t 0 = all logging turned off\n"
    << "\t\t 1 = log only error messages\n"
    << "\t\t 2 = (default) log error and warning messages\n"
    << "\t\t 3 = log error, warning and informational messages\n"
    << "\t\t 4 = log everything, including development debug messages\n"
    << "\t-b,--bind URI\t\t\tBind to an URI\n"
    << "\t\t URI = PREFIX://IP:PORT\n"
    << "\t\t URI = IP:PORT, default to dns:// prefix\n"
    << "\t\t URI = IP, default to dns:// prefix and port 443\n"
    << endl;
}

int main (int argc, char* argv[]) {
  int c;
  extern char *optarg;
  int option_index = 0;
  string bind_addr = "localhost:50051";
  string username, password;
  Log();
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
    {"bind", required_argument, 0, 'b'}, //insecure mode
    {0, 0, 0, 0}
  };

  /*
   * An option character followed by ('') indicates no argument
   * An option character followed by (‘:’) indicates a required argument.
   * An option character is followed by (‘::’) indicates an optional argument.
   * Here: no argument after (h,f) ; mandatory argument after (p,u,l,b)
   */
  while ((c = getopt_long(argc, argv, "hfl:p:u:c:k:b:", long_options, &option_index))
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
          cerr << "Please specify a string with username option\n"
            << "Ex: -u USERNAME" << endl;
          exit(1);
        }
        break;
      case 'p':
        if (optarg) {
          cxt->SetAuthType(USERPASS);
          cxt->SetPassword(string(optarg));
        } else {
          cerr << "Please specify a string with password option\n"
            << "Ex: -p PASSWORD" << endl;
          exit(1);
        }
        break;
      case 'k':
        if (optarg) {
          cxt->SetKeyPath(string(optarg));
        } else {
          cerr << "Please specify a string with private key path\n"
            << "Ex: --private-key KEY_PATH" << endl;
          exit(1);
        }
        break;
      case 'c':
        if (optarg) {
          cxt->SetCertsPath(string(optarg));
        } else {
          cerr << "Please specify a string with chain certs path\n"
            << "Ex: --cert-chain CERTS_PATH" << endl;
          exit(1);
        }
        break;
      case 'l':
        if (optarg) {
          Log::setLevel(atoi(optarg));
        } else {
          cerr << "Please specify a log level" << endl;
        }
        break;
      case 'b':
        if (optarg) {
          bind_addr = optarg;
        } else {
          cerr << "Please specify a string with binding address" << endl;
          exit(1);
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
      cerr << "Both private key and certificate required" << endl;
      exit(1);
    }
    cout << "Initiate a TLS connection" << endl;
  }

  if (cxt->GetAuthType() == AuthType::USERPASS) {
    if (cxt->GetUsername().empty() || cxt->GetPassword().empty()) {
      cerr << "Both username and password required" << endl;
      exit(1);
    }
  }

  RunServer(cxt, bind_addr);

  return 0;
}
