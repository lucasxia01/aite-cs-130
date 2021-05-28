#include "meme_handler.h"

http::response MemeGenHandler::handle_request(const http::request &req) const {
  std::string body = req.body();
  if (req.method() != http::verb::post ||
      req.find(http::field::content_type) == req.end()) {
    return show_error_page(http::status::bad_request, "Invalid upload request");
  }

  std::string content_type = req.at(http::field::content_type).to_string();

  std::stringstream ss;
  ss << req;
  std::string req_s = ss.str();

  if (content_type.find("multipart/form-data") == std::string::npos) {
    return show_error_page(http::status::unsupported_media_type,
                           "Invalid upload content type");
  }

  std::string boundary;
  std::smatch match_boundary;
  std::regex r_boundary("boundary=([-]{10,}[0-9]{10,})");

  // Find initial boundary provided by http request
  if (std::regex_search(req_s, match_boundary, r_boundary)) {
    boundary = match_boundary[0];
    boost::trim(boundary);
    boundary = boundary.substr(boundary.find('-'));
  } else {
    return show_error_page(http::status::bad_request, "Invalid multipart form");
  }

  std::stringstream ss_body;
  ss_body << req.body();
  std::string ss_body_s = ss_body.str();

  std::string curr_line;
  std::size_t boundary_count = 0;
  int parse_state = 0;
  int invalid_parse = false;

  std::stringstream ss_part_header;
  std::stringstream ss_part_body;

  bool crlf_end = false;

  /*
  Parse states
  0: Read in multipart opening boundary
  1: Read in header
  2: Read in body
  3: Read in multipart closing boundary

  If there is anything other than a single part
  in the body then it is considered invalid
  */
  while (std::getline(ss_body, curr_line)) {
    if (parse_state == 0) {
      if (curr_line != "--" + boundary + "\r") {
        invalid_parse = true;
        break;
      } else {
        parse_state = 1;
      }
    } else if (parse_state == 1) {
      // Reading in header
      if (curr_line == "--" + boundary + "--\r") {
        parse_state = 3;
      } else {
        if (curr_line == "\r" && crlf_end) {
          // Two CRLFs transition to body
          parse_state = 2;
        } else {
          ss_part_header << curr_line << "\n";
        }
      }
    } else if (parse_state == 2) {
      // Reading in body
      if (curr_line == "--" + boundary + "--\r") {
        parse_state = 3;
      } else {
        ss_part_body << curr_line << "\n";
      }
    } else if (parse_state == 3) {
      invalid_parse = true;
      break;
    }
    crlf_end = (!curr_line.empty() && curr_line.back() == '\r');
  }

  if (invalid_parse || parse_state != 3) {
    return show_error_page(http::status::bad_request,
                           "Invalid multipart request");
  }

  std::string b_header = ss_part_header.str();
  std::string curr_content_type;
  std::smatch match_curr_content_type;
  std::regex r_content_type("\\b(Content-Type: )(.*)");
  if (std::regex_search(b_header, match_curr_content_type, r_content_type)) {
    curr_content_type = match_curr_content_type[0];
    boost::trim(curr_content_type);
  }
  if (curr_content_type.find("image/") == std::string::npos) {
    return show_error_page(http::status::bad_request, "Bad file type");
  }

  curr_content_type = curr_content_type.substr(
      curr_content_type.find("image/") + 6); // image/jpeg -> jpeg
  std::filesystem::path path{"./memes"};
  boost::uuids::uuid u;
  path /= boost::uuids::to_string(u) + "." + curr_content_type;
  std::filesystem::create_directories(path.parent_path());
  std::ofstream ofs(path);
  ofs << ss_part_body.str();
  ofs.close();

  http::response resp;
  resp.result(http::status::ok);
  resp.set(http::field::content_type, "text/plain");
  resp.body() = "pls make some memes\n";
  resp.prepare_payload();
  return resp;
}