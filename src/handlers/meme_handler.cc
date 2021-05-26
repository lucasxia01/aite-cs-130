#include "meme_handler.h"

http::response
MemeGenHandler::handle_request(const http::request &req) const {
  http::response resp;
  resp.result(http::status::ok);
  resp.set(http::field::content_type, "text/plain");
  resp.body() = "pls make some memes\n";
  resp.prepare_payload();
  return resp;
}