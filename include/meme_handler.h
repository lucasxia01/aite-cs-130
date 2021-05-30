#ifndef MEME_HANDLER_H
#define MEME_HANDLER_H

#include "request_handler.h"
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <regex>

/**
 * Handler that's used for testing multithreading
 */
class MemeGenHandler : public RequestHandler {
public:
  MemeGenHandler() {}
  http::response handle_request(const http::request &req) const;

private:
  bool parse(std::stringstream &, std::stringstream &, std::stringstream &,
             std::string boundary) const;
  bool is_boundary(std::string boundary, std::string line) const;
};

#endif // MEME_HANDLER_H