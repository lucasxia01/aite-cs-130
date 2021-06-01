#ifndef MEME_HANDLER_H
#define MEME_HANDLER_H

#include "request_handler.h"
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <map>
#include <regex>

/**
 * Handler that's used for testing multithreading
 */
class MemeHandler : public RequestHandler {
public:
  MemeHandler(const std::string &location, const NginxConfig &config);
  http::response handle_request(const http::request &req) const;
  void initMeme(server *parent_server);

private:
  bool parse(std::stringstream &, std::stringstream &, std::stringstream &,
             std::string boundary) const;
  bool is_boundary(std::string boundary, std::string line) const;
  http::response generate_meme(const http::request &req) const;
  http::response create_meme(const http::request &req) const;
  http::response browse_memes(const http::request &req) const;
  server *parent_server_;
};

#endif // MEME_HANDLER_H