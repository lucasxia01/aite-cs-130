#include "response.h"
#include <fstream>
#include <map>
#include <request.h>
#include <set>
#include <sstream>
#include <string>

class RequestHandler {
protected:
  const int MAX_RESPONSE_HEADER_SIZE = 100;

public:
  RequestHandler() {}
  virtual response generateResponse(response::status_type status,
                                    const http::server3::request &req) = 0;
};

class EchoRequestHandler : public RequestHandler {
  std::set<std::string> roots;

public:
  EchoRequestHandler(std::set<std::string> roots) : roots(roots) {}
  bool containsRoot(std::string root) {
    return roots.find(root) != roots.end();
  }
  response generateResponse(response::status_type status,
                            const http::server3::request &req);
};

class StaticFileRequestHandler : public RequestHandler {

public:
  StaticFileRequestHandler(std::map<std::string, std::string> root_to_base_dir)
      : root_to_base_dir(root_to_base_dir) {}
  bool containsRoot(std::string root) {
    return root_to_base_dir.find(root) != root_to_base_dir.end();
  }
  response generateResponse(response::status_type status,
                            const http::server3::request &req);

private:
  std::map<std::string, std::string> root_to_base_dir;

  void parseUri(std::string uri, std::string &root, std::string &path,
                std::string &content_type);
};
