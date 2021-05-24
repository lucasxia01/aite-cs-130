#ifndef SERVER_H
#define SERVER_H

#include <array>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <map>
#include <optional>
#include <string>

#include "logger.h"
#include "request_handler.h"
#include "session.h"
#include "tcp_socket_wrapper.h"

using namespace boost::asio::ip;

template <class TSocket> class session;
class RequestHandler;
class NginxConfig;

class server {
public:
  server(int thread_pool_size, const NginxConfig &config);
  void run();

  // starts a session
  session<tcp_socket_wrapper> *start_accept();
  // callback function for start_accept
  void handle_accept(session<tcp_socket_wrapper> *new_session,
                     const boost::system::error_code &error);

  void handle_stop();

  const RequestHandler *get_request_handler(std::string request_uri) const;

  void log_request(std::pair<std::string, http::status>);

  const std::map<std::pair<std::string, http::status>, int> get_requests();

  const std::map<std::string, std::vector<std::string>> get_prefix_map();

  ~server();
  std::map<const std::string, const RequestHandler *> location_to_handler_;
  std::string get_handler_type(const RequestHandler *ptr);
  boost::asio::io_service& get_io_service();

private:
  boost::asio::io_service io_service_;
  std::unique_ptr<tcp::acceptor> acceptor_;
  boost::asio::signal_set signals_;
  int thread_pool_size_;

  static const std::array<std::string, 7> handler_types;
  std::map<std::pair<std::string, http::status>, int> requests_;
  std::map<std::string, std::vector<std::string>> handler_to_prefixes_;
  std::map<std::string, std::vector<const RequestHandler *>> type_to_handler_;
  void getHandlers(const NginxConfig &config);
  void create_and_add_handler(std::string type, const std::string &location,
                              const NginxConfig &config);
};

#endif // SERVER_H