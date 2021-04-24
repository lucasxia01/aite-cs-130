#include "logger.h"

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;

void Logger::init() {
  std::string format_str = "[%TimeStamp%] [%ThreadID%] [%ProcessID%] "
                           "[%LineID%] [%Severity%]: %Message%";
  logging::register_simple_formatter_factory<logging::trivial::severity_level,
                                             char>("Severity");
  logging::add_file_log(
      keywords::file_name = "./logs/%d_%m_%Y_logs_%N.log", // Log file name format
      keywords::rotation_size = 10 * 1024 * 1024,   // Every 10 MiB
      keywords::format = format_str.c_str(),
      keywords::time_based_rotation =
          sinks::file::rotation_at_time_point(0, 0, 0),
      keywords::auto_flush = true);
  logging::add_console_log(std::clog, keywords::format = format_str.c_str());
  logging::add_common_attributes();
}
