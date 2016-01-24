#include "utilities.hpp"

// LOGGING GLOBAL SETTINGS
// These are the default values.
// The base log filename should be changed before use, in the application code.
// The current log verbosity should also be set in application code (perhaps via command line param),
// but note that it can be changed on the fly as needed.
// g_ss is available to (re)use when logging.
//
// LOGGING EXAMPLE:
//
//      g_ss.str(std::string());
//      g_ss << "header: \r\n" << str_header;
//      log(LV_DEBUG,g_ss.str());
//
string g_base_log_filename = "app";
LOG_TO_FILE_VERBOSITY g_current_log_verbosity = LV_INFO;
stringstream g_ss;
