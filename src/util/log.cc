#include "log.hh"

std::string logging_color = "0";
std::string service_name = "(undefined)";
bool        logging_verbose = false;
FILE*       logging_dest = stdout;