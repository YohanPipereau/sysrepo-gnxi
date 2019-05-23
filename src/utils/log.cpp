/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <iostream>

#include "log.h"

static void _boost_set_log_level(int lvl)
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

  logging::core::get()->set_filter(
    logging::trivial::severity >= l
  );
}


Log::Log(int lvl)
{
  _boost_set_log_level(lvl);
}

void Log::setLevel(int lvl)
{
  _boost_set_log_level(lvl);
}
