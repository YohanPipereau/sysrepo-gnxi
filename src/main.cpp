/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <iostream>
#include <memory>
#include <chrono>
#include <getopt.h>

#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include "gnmi/gnmi.h"
#include <security/authentication.h>
#include <utils/log.h>

using namespace std;

void RunServer(string bind_addr, shared_ptr<ServerCredentials> cred)
{
  ServerBuilder builder;
  GNMIService gnmi("gnmi"); //gNMI Service

  builder.AddListeningPort(bind_addr, cred);
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
  AuthBuilder auth;

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
      case '?': //help
      case 'h':
        show_usage(argv[0]);
        exit(0);
        break;
      case 'u': //username
        auth.setUsername(string(optarg));
        break;
      case 'p': //password
        auth.setPassword(string(optarg));
        break;
      case 'k': //server private key
        auth.setKeyPath(string(optarg));
        break;
      case 'c': //server certificate
        auth.setCertPath(string(optarg));
        break;
      case 'r': //CA/root certificate
        auth.setRootCertPath(string(optarg));
        break;
      case 'l': //log level
        Log::setLevel(atoi(optarg));
        break;
      case 'b': //binding address
        bind_addr = optarg;
        break;
      case 'f': //force insecure connection
        auth.setInsecure(true);
        break;
      default: /* You won't get there */
        exit(1);
    }
  }

  RunServer(bind_addr, auth.build());

  return 0;
}
