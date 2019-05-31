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
    << "\t-k,--private-key PRIVATE_KEY\tpath to server TLS private key\n"
    << "\t-c,--cert CERTIFICATE\tpath to server TLS certificate\n"
    << "\t-r,--ca CERTIFICATE\tpath to root certificate/CA certificate\n"
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
    {"cert", required_argument, 0, 'c'}, //certificate chain
    {"ca", required_argument, 0, 'r'}, //certificate chain
    {"force-insecure", no_argument, 0, 'f'}, //insecure mode
    {"bind", required_argument, 0, 'b'}, //insecure mode
    {0, 0, 0, 0}
  };

  /*
   * An option character followed by ('') indicates no argument
   * An option character followed by (‘:’) indicates a required argument.
   * An option character is followed by (‘::’) indicates an optional argument.
   * Here: no argument after (h,f) ; mandatory argument after (p,u,l,b,c,k,r)
   */
  while ((c = getopt_long(argc, argv, "hfl:p:u:c:k:r:b:", long_options, &option_index))
         != -1) {
    switch (c)
    {
      case 'h': //help
        show_usage(argv[0]);
        exit(0);
        break;
      case 'u': //username
        if (optarg) {
          cxt->SetAuthType(USERPASS);
          cxt->SetUsername(string(optarg));
        } else {
          cerr << "Please specify a string with username option\n"
            << "Ex: -u USERNAME" << endl;
          exit(1);
        }
        break;
      case 'p': //password
        if (optarg) {
          cxt->SetAuthType(USERPASS);
          cxt->SetPassword(string(optarg));
        } else {
          cerr << "Please specify a string with password option\n"
            << "Ex: -p PASSWORD" << endl;
          exit(1);
        }
        break;
      case 'k': //server private key
        if (optarg) {
          cxt->SetKeyPath(string(optarg));
        } else {
          cerr << "Please specify a string with private key path\n"
            << "Ex: --private-key KEY_PATH" << endl;
          exit(1);
        }
        break;
      case 'c': //server certificate
        if (optarg) {
          cxt->SetCertPath(string(optarg));
        } else {
          cerr << "Please specify a string with cert path\n"
            << "Ex: --cert CERT" << endl;
          exit(1);
        }
        break;
      case 'r': //CA/root certificate
        if (optarg) {
          cxt->SetRootCertPath(string(optarg));
        } else {
          cerr << "Please specify a string with CA path\n"
            << "Ex: --ca CERT" << endl;
          exit(1);
        }
        break;
      case 'l': //log level
        if (optarg) {
          Log::setLevel(atoi(optarg));
        } else {
          cerr << "Please specify a log level" << endl;
        }
        break;
      case 'b': //binding address
        if (optarg) {
          bind_addr = optarg;
        } else {
          cerr << "Please specify a string with binding address" << endl;
          exit(1);
        }
        break;
      case 'f': //force insecure connection
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
    if (cxt->GetKeyPath().empty() || cxt->GetCertPath().empty()) {
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
