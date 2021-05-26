#include "status_handler.h"

#include <map>

StatusRequestHandler::StatusRequestHandler(const std::string &location,
                                           const NginxConfig &config)
    : parent_server_(nullptr) {}
void StatusRequestHandler::initStatus(server *parent_server) {
  parent_server_ = parent_server;
}

http::response
StatusRequestHandler::handle_request(const http::request &req) const {
  std::ostringstream ss;
  std::ostringstream temp;
  int num_req = 0;

  if (parent_server_ == nullptr) {
    LOG_FATAL << "Status Handler was not correctly initialized";
    return show_error_page(http::status::internal_server_error,
                           "Status Handler was not correctly initialized");
  } else {
    const std::map<std::pair<std::string, http::status>, int> requests_ =
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
          ".code1{ background-color: cyan } "
          ".code2{ background-color: green } "
          ".code3{ background-color: yellow } "
          ".code4{ background-color: red } "
          ".code5{ background-color: orange } "
          "td,th{ text-align: center; padding-right: 12px; border: 1px solid "
          "black } "
          "td{ max-width: 200px; word-wrap: break-word } "
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
    // display table of served requests and response codes
    if (requests_.size()) {
      temp << "<table style = \"width:100%\">"
              "<tr><th>Requested URL</th><th>Response Status "
              "Code</th><th>Number of Requests</th></tr>";
      for (auto req : requests_) {
        temp << "<tr><td>" << req.first.first
             << "</td>"
                "<td class="
             << "code"
             << (int)((int)req.first.second /
                      pow(10, (int)log10((int)req.first.second)))
             << ">" << (int)req.first.second << " " << req.first.second
             << "</td>"
             << "<td>" << req.second << "</td></tr>";
        num_req += req.second;
      }
      temp << "</table>";
    }
    ss << "<b>Total Number of Requests Served : </b>" << num_req << "<br><br>";
    ss << temp.str();
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
