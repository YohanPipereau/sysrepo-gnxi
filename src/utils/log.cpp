/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <iostream>

#include "log.h"

Log::Log(int lvl)
{
  logging::trivial::severity_level l;

  switch (lvl) {
    case 0: //no log
      l = logging::trivial::fatal;
      break;
    case 1: //only error message
      l = logging::trivial::error;
      break;
    case 2: //log error and warning
      l = logging::trivial::warning;
      break;
    case 3: //log error, warning, and informational
      l = logging::trivial::info;
      break;
    case 4: //log error, warning, informational and debug
      l = logging::trivial::debug;
      break;
    default:
      std::cerr << "Unused log level" << std::endl;
      exit(1);
  }

  std::cout << "Launch with " << logging::trivial::to_string(l) << std::endl;

  logging::core::get()->set_filter(
    logging::trivial::severity >= logging::trivial::info
  );
}
