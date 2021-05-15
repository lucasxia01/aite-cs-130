#include "request_handler.h"
StatusRequestHandler::StatusRequestHandler(const std::string &location,
                                           const NginxConfig &config)
    : parent_server_(nullptr) {}
void StatusRequestHandler::initStatus(server *parent_server) {
  parent_server_ = parent_server;
}

http::response
StatusRequestHandler::handle_request(const http::request &req) const {
  std::ostringstream ss;

  if (parent_server_ == nullptr) {
    LOG_FATAL << "Status Handler was not correctly initialized";
    return show_error_page(http::status::internal_server_error,
                           "Status Handler was not correctly initialized");
  } else {
    const std::vector<std::pair<std::string, http::status>> requests_ =
        parent_server_->get_requests();
    const std::map<std::string, std::vector<std::string>> handler_to_prefixes_ =
        parent_server_->get_prefix_map();

    ss << "<!DOCTYPE html><html><link rel ='preconnect' "
          "href='https://fonts.gstatic.com'>"
          "<link href"
          "='https://fonts.googleapis.com/css2?family=Handlee&display=swap' "
          "rel='stylesheet'>"
          "<style> body{ font-family: 'Handlee', cursive; font-size: 20px; "
          "display: block } "
          ".code2{ background-color: green } "
          ".code4, .code5{ background-color: red } "
          ".code3{ background-color: cyan } "
          "td,th{ text-align: center; padding-right: 12px; border: 1px solid "
          "black } "
          "table{ border: 1px solid black } "
          "#handlers-list { display: flex; flex-wrap: wrap } "
          ".handler{ margin-right:12px } "
          "</style><head><title>Status Page</title></head><body><h1>Status "
          "Page</h1>";
    // display list of existing handlers
    if (handler_to_prefixes_.size()) {
      ss << "<div id='handlers-list'>";
      for (auto const &pair : handler_to_prefixes_) {
        ss << "<div class=handler><b>" << pair.first << " Handler(s)</b><ol>";
        for (auto prefix : pair.second) {
          ss << "<li>" << prefix << "</li>";
        }
        ss << "</ol></div>";
      }
      ss << "</div>";
    }
    ss << "<b>Number of Requests Served : </b>" << requests_.size()
       << "<br><br>";
    // display table of served requests and response codes
    if (requests_.size()) {
      ss << "<table style = \"width:100%\">"
            "<tr><th>Requested URL</th><th>Response Status Code</th></tr>"
            "<tr>";
      for (auto req : requests_) {
        ss << "<tr><td>" << req.first
           << "</td>"
              "<td class="
           << "code"
           << (int)((int)req.second / pow(10, (int)log10((int)req.second)))
           << ">" << (int)req.second << " " << req.second << "</td></tr>";
      }
      ss << "</table>";
    }
    ss << "</body></html>";
  }
  std::string response_content = ss.str();

  http::response resp;
  resp.result(http::status::ok);
  resp.set(http::field::content_type, "text/html");
  resp.content_length(response_content.length());
  resp.body() = response_content;
  return resp;
}
