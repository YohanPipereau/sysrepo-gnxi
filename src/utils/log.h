/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _LOG_H
#define _LOG_H

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace logging = boost::log;

/*
 * Pick your severity
 * BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
 * BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
 * BOOST_LOG_TRIVIAL(info) << "An informational severity message";
 * BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
 * BOOST_LOG_TRIVIAL(error) << "An error severity message";
 * BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";
*/
class Log {
  public:
    /*
     * lvl 0 : fatal
     * lvl 1 : error
     * lvl 2 : warning
     * lvl 3 : info
     * lvl 4 : debug
     */
    Log(int lvl = 3); //default to 'info' log
    ~Log() {}

    static void setLevel(int lvl);
};

#endif // _LOG_H
