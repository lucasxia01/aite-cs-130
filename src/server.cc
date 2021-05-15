#include "server.h"

server::server(boost::asio::io_service &io_service, const NginxConfig &config)
    : io_service_(io_service) {
  int port_num = getPortNumber(config);
  if (port_num == -1) {
    LOG_ERROR << "Failed to parse port number";
    return;
  }
  LOG_INFO << "Successfully parsed port number: " << port_num;

  acceptor_ = std::make_unique<tcp::acceptor>(
      io_service, tcp::endpoint(tcp::v4(), port_num));
  getHandlers(config);
  start_accept();
  LOG_INFO << "Running server on port number: " << port_num;
}

// parses config for location blocks in the server block
void server::getHandlers(const NginxConfig &config) {
  LOG_DEBUG << "Parsing config file for request handlers";
  for (const auto &statement : config.statements_) {
    // check if its a block
    if (statement->child_block_) {
      std::vector<std::string> tokens = statement->tokens_;
      // if it is server block, we want to look in it
      if (tokens.size() == 1 && tokens[0] == "server") {
        NginxConfig server_config = *(statement->child_block_);
        // now look for location block
        for (const auto &server_statement : server_config.statements_) {
          if (server_statement->child_block_) {
            std::vector<std::string> block_tokens = server_statement->tokens_;
            if (block_tokens.size() == 3 && block_tokens[0] == "location") {
              LOG_DEBUG << "location " << block_tokens[1] << ", handler "
                        << block_tokens[2];
              NginxConfig location_config = *(server_statement->child_block_);
              HandlerType handler_type;
              if (block_tokens[2] == "EchoHandler") {
                handler_type = HANDLER_ECHO;
              } else if (block_tokens[2] == "StaticHandler") {
                handler_type = HANDLER_STATIC_FILE;
              } else if (block_tokens[2] == "ReverseProxyHandler") {
                handler_type = HANDLER_REVERSE_PROXY;
              } else if (block_tokens[2] == "NotFoundHandler") {
                handler_type = HANDLER_NOT_FOUND;
              } else if (block_tokens[2] == "StatusHandler") {
                handler_type = HANDLER_STATUS;
              } else {
                LOG_ERROR << "Handler " << block_tokens[2] << " does not exist";
              }
              create_and_add_handler(handler_type, block_tokens[1],
                                     location_config);
            }
          }
        }
      }
    }
  }
}

void server::create_and_add_handler(HandlerType type,
                                    const std::string &location,
                                    const NginxConfig &config) {
  std::string loc = convertToAbsolutePath(location);
  LOG_DEBUG << "Location is " << loc;
  const RequestHandler *handler;
  std::string name;
  switch (type) {
  case HANDLER_ECHO:
    handler = new EchoRequestHandler(loc, config);
    name = "Echo";
    break;
  case HANDLER_STATIC_FILE:
    handler = new StaticFileRequestHandler(loc, config);
    name = "Static";
    break;
  case HANDLER_REVERSE_PROXY:
    handler = new ReverseProxyRequestHandler(loc, config);
    name = "Reverse Proxy";
    break;
  case HANDLER_NOT_FOUND:
    handler = new NotFoundRequestHandler(loc, config);
    name = "404";
    break;
  case HANDLER_STATUS:
    StatusRequestHandler *temp = new StatusRequestHandler(loc, config);
    temp->initStatus(this);
    handler = temp;
    name = "Status";
    break;
  }
  handler_to_prefixes_[name].push_back(location);
  location_to_handler_[loc] = handler;
}
session<tcp_socket_wrapper> *server::start_accept() {
  LOG_DEBUG << "Starting accept";
  session<tcp_socket_wrapper> *new_session =
      new session<tcp_socket_wrapper>(io_service_, this);
  LOG_DEBUG << "Waiting to accept new connection...";
  acceptor_->async_accept((new_session->socket()).get_socket(),
                          boost::bind(&server::handle_accept, this, new_session,
                                      boost::asio::placeholders::error));
  return new_session;
}

void server::handle_accept(session<tcp_socket_wrapper> *new_session,
                           const boost::system::error_code &error) {
  if (!error) {
    LOG_INFO << "Accepted connection with address: "
             << (new_session->socket()).get_endpoint_address();
    new_session->start();
  } else {
    LOG_ERROR << "Error when accepting socket connection: " << error.message();
    delete new_session;
  }

  start_accept();
}

void server::log_request(std::string request_uri, http::status status_code) {
  requests_.push_back(std::make_pair(request_uri, status_code));
}

const std::vector<std::pair<std::string, http::status>> server::get_requests() {
  return requests_;
}

const std::map<std::string, std::vector<std::string>> server::get_prefix_map() {
  return handler_to_prefixes_;
}

/**
 * Get the appropriate request handler (echo or static file) based on the
 * request URI. If neither valid echo nor valid static file roots are found,
 * return null.
 **/
const RequestHandler *
server::get_request_handler(std::string request_uri) const {
  boost::filesystem::path uri(convertToAbsolutePath(request_uri));
  // look through all path prefixes and check if any match a valid location
  // prioritizing longest path prefix
  LOG_DEBUG << "Parsed request location: " << uri.string();
  while (uri.has_parent_path()) {
    if (location_to_handler_.find(uri.string()) != location_to_handler_.end()) {
      return location_to_handler_.at(uri.string());
    }
    uri = uri.parent_path();
  }
  LOG_DEBUG << "Ending uri: " << uri.string();
  if (location_to_handler_.find("") == location_to_handler_.end()) {
    LOG_ERROR << "404 Handler not found at root /";
    return nullptr;
  }
  return location_to_handler_.at("");
}
server::~server() {
  for (auto &entry : location_to_handler_) {
    delete entry.second;
  }
}
