#include "meme_handler.h"

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <cctype>

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
  std::regex r_boundary("boundary=([-]{4,}[a-zA-Z0-9]{10,})");

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

  // Start parsing the multipart form

  // If the body doesn't consist of exactly four parts sectioned off by the
  // boundaries (and no content outside of the boundaries), then it is
  // considered invalid

  std::string line;
  if (!std::getline(ss_body, line) || line != "--" + boundary + "\r") {
    return show_error_page(http::status::bad_request, "Invalid meme request");
  }

  std::stringstream ss_template_style_header;
  std::stringstream ss_template_style_body;
  if (!this->parse(ss_body, ss_template_style_header, ss_template_style_body,
                   boundary)) {
    return show_error_page(http::status::bad_request, "Invalid meme request");
  }

  std::stringstream ss_base_image_header;
  std::stringstream ss_base_image_body;
  if (!this->parse(ss_body, ss_base_image_header, ss_base_image_body,
                   boundary)) {
    return show_error_page(http::status::bad_request, "Invalid meme request");
  }

  std::stringstream ss_captions_top_header;
  std::stringstream ss_captions_top_body;
  if (!this->parse(ss_body, ss_captions_top_header, ss_captions_top_body,
                   boundary)) {
    return show_error_page(http::status::bad_request, "Invalid meme request");
  }

  std::stringstream ss_captions_bottom_header;
  std::stringstream ss_captions_bottom_body;
  if (!this->parse(ss_body, ss_captions_bottom_header, ss_captions_bottom_body,
                   boundary)) {
    return show_error_page(http::status::bad_request, "Invalid meme request");
  }

  if (std::getline(ss_body, line)) {
    return show_error_page(http::status::bad_request, "Invalid meme request");
  }

  std::string b_header = ss_base_image_header.str();
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
  boost::uuids::uuid u = boost::uuids::random_generator()();
  path /= boost::uuids::to_string(u) + "." + curr_content_type;
  std::filesystem::create_directories(path.parent_path());
  std::ofstream ofs(path);
  ofs << ss_base_image_body.str();
  ofs.close();

  std::string style = ss_template_style_body.str();
  std::ostringstream ss_resp;
  ss_resp << "<!DOCTYPE html><html><head><link rel='preconnect' "
             "href='https://fonts.gstatic.com'>"
             "<link href"
             "='https://fonts.googleapis.com/"
             "css2?family=Odibee+Sans&family=Oswald:wght@700&family=Sigmar+One&"
             "display=swap'</head>"
             "<body><div class=template><div class='top-text captions'>"
          << ss_captions_top_body.str()
          << "</div>"
             "<div class='bottom-text captions'>"
          << ss_captions_bottom_body.str()
          << "</div></div><br></body>"
             "<style>"
             ".template{width: 20vw;height: 20vw;margin:1vw;font-family: "
             "'Oswald', sans-serif;text-transform: "
             "uppercase;background-image:url("
          << path.string()
          << ");background-position: center; background-size:cover; color: "
             "white;font-size: "
             "2vw;-webkit-text-stroke: 1px black;position: "
             "relative;align-content: center;}"
             ".top-text{height: 20%;width: 100%;position: "
             "absolute;top:0;text-align: center;}"
             ".bottom-text{height: 20%;width:100%;position: absolute;bottom: "
             "0;text-align: center;}";
  enum template_style { white_font = 1, black_background = 2, black_font = 3 };
  style.erase(std::remove_if(style.begin(), style.end(), ::isspace),
              style.end());
  int chosen_style = stoi(style);
  switch (chosen_style) {
  case white_font:
    break;
  case black_background:
    ss_resp << ".captions{background-color:black}";
    break;
  case black_font:
    ss_resp << ".captions{color:black}";
    break;
  default:
    LOG_DEBUG << "style[0] was: " << style[0];
    return show_error_page(http::status::bad_request, "Invalid meme request");
  }
  ss_resp << "</style></body></html>";
  std::string response_content = ss_resp.str();

  http::response resp;
  resp.result(http::status::ok);
  resp.set(http::field::content_type, "text/html");
  resp.content_length(response_content.length());
  resp.body() = response_content;
  return resp;
}

bool MemeGenHandler::parse(std::stringstream &ss_body,
                           std::stringstream &ss_part_header,
                           std::stringstream &ss_part_body,
                           std::string boundary) const {
  std::string curr_line;
  enum parse_state { reading_header, reading_body, reached_boundary };
  parse_state state = reading_header;
  bool crlf_end = false;

  /*
  Parse states
  1: Read in header
  2: Read in body
  3: Read in multipart closing boundary
  */
  while (state != reached_boundary && std::getline(ss_body, curr_line)) {
    // First boundary line marking beginning of section
    if (state == reading_header) {
      // Reading in header
      if (this->is_boundary(boundary, curr_line)) {
        state = reached_boundary;
      } else {
        if (curr_line == "\r" && crlf_end) {
          // Two CRLFs transition to body
          state = reading_body;
        } else {
          ss_part_header << curr_line << "\n";
        }
      }
    } else if (state == reading_body) {
      // Reading in body
      if (this->is_boundary(boundary, curr_line)) {
        state = reached_boundary;
      } else {
        ss_part_body << curr_line << "\n";
      }
    }
    crlf_end = (!curr_line.empty() && curr_line.back() == '\r');
  }

  return (state == reached_boundary);
}

bool MemeGenHandler::is_boundary(std::string boundary, std::string line) const {
  return line == "--" + boundary + "--\r" || line == "--" + boundary + "\r";
}