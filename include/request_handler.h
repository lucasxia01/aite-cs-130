#include <map>
#include <request.h>
#include <set>
#include <sstream>
#include <string>

enum status_type { OK = 200, BAD_REQUEST = 400 };

class RequestHandler {
protected:
  const int MAX_RESPONSE_HEADER_SIZE = 100;
  const std::string INVALID_REQUEST_MESSAGE = "Invalid request\n";

public:
  RequestHandler() {}
  virtual std::string generateResponse(status_type status,
                                       const http::server3::request &req) = 0;
};

class EchoRequestHandler : public RequestHandler {
  std::set<std::string> roots;

public:
  EchoRequestHandler(std::set<std::string> roots)
      : roots(roots) {}
  bool containsRoot(std::string root) {
    return roots.find(root) != roots.end();
  }
  std::string generateResponse(status_type status,
                               const http::server3::request &req);
};

class StaticFileRequestHandler : public RequestHandler {
  std::map<std::string, std::string> root_to_base_dir;

public:
  StaticFileRequestHandler(std::map<std::string, std::string> root_to_base_dir)
      : root_to_base_dir(root_to_base_dir) {}
  bool containsRoot(std::string root) {
    return root_to_base_dir.find(root) != root_to_base_dir.end();
  }
  std::string generateResponse(status_type status,
                               const http::server3::request &req);
};
