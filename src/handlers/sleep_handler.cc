#include "sleep_handler.h"

http::response SleepHandler::handle_request(const http::request &req) const {
  boost::this_thread::sleep_for(boost::chrono::milliseconds(5000));
  http::response resp;
  resp.result(http::status::ok);
  resp.set(http::field::content_type, "text/plain");
  resp.body() = "Slept for 5 seconds!\n";
  resp.prepare_payload();
  return resp;
}