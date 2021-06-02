#include "meme_handler.h"

#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <cctype>
#include <fstream>

MemeHandler::MemeHandler(const std::string &location, const NginxConfig &config)
    : parent_server_(nullptr) {
  std::vector<std::string> values = configLookup(config, {}, "root");
  if (values.size() != 1) {
    LOG_FATAL << "Invalid number of root specifiers for " << location
              << ", only one root should be listed per location";
  }
  this->root = values[0];
  this->root =
      (this->root[0] == '"' && this->root[this->root.length() - 1] == '"')
          ? this->root.substr(1, this->root.length() - 2)
          : this->root;
}

void MemeHandler::initMeme(server *parent_server) {
  parent_server_ = parent_server;
}

http::response MemeHandler::handle_request(const http::request &req) const {
  if (parent_server_ == nullptr) {
    LOG_FATAL << "Meme Handler was not correctly initialized";
    return show_error_page(http::status::internal_server_error,
                           "Status Handler was not correctly initialized");
  } else {
    if (req.target().to_string() == "/meme/generate") {
      return generate_meme(req);
    } else if (req.target().to_string() == "/meme/create") {
      return create_meme(req);
    } else if (req.target().to_string() == "/meme/browse") {
      return browse_memes(req);
    } else {
      return show_error_page(http::status::not_found,
                             req.target().to_string() + " page not found");
    }
  }
}

http::response MemeHandler::generate_meme(const http::request &req) const {
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

  std::string b_body = ss_base_image_body.str();

  if (curr_content_type.find("image/") == std::string::npos) {
    return show_error_page(http::status::bad_request, "Bad file type");
  }

  curr_content_type = curr_content_type.substr(
      curr_content_type.find("image/") + 6); // image/jpeg -> jpeg
  LOG_INFO << "[MemeMetric][" << curr_content_type << "]"
           << "[" << b_body.length() << "]";
  std::filesystem::path path = this->root;
  try {
    boost::uuids::uuid u = boost::uuids::random_generator()();
    path /= boost::uuids::to_string(u) + "." + curr_content_type;
    std::filesystem::create_directories(path.parent_path());
    std::ofstream ofs(path);
    ofs << b_body;
    ofs.close();
  } catch (std::exception e) {
    return show_error_page(http::status::internal_server_error,
                           "Could not write meme to configured filesystem");
  }

  std::string top = ss_captions_top_body.str();
  std::string bottom = ss_captions_bottom_body.str();
  std::string style = ss_template_style_body.str();
  style.erase(std::remove_if(style.begin(), style.end(), ::isspace),
              style.end());
  std::vector<std::string> selections;
  selections.push_back(top);
  selections.push_back(bottom);
  selections.push_back(style);

  std::string img_url = +"." + path.string();
  parent_server_->log_meme(img_url, selections);
  std::ostringstream ss_resp;
  ss_resp
      << "<!DOCTYPE html><html><head><link rel='preconnect' "
         "href='https://fonts.gstatic.com'>"
         "<link href"
         "='https://fonts.googleapis.com/"
         "css2?family=Odibee+Sans&family=Oswald:wght@700&family=Sigmar+One&"
         "display=swap' rel='stylesheet'></head>"
         "<body style=\"background-color:lightblue; font-family: 'Sigmar One', "
         "cursive;color: navy\"><div "
         "class='page-layout'><div class=column></div><div class='page-content "
         "column'><h1>Generated Meme</h1><div "
         "class=template><div class='top-text captions'>"
      << top
      << "</div>"
         "<div class='bottom-text captions'>"
      << bottom
      << "</div></div><form method='post' action='/meme/browse'><button "
         "style='position: "
         "absolute; left: 0'>Browse "
         "Memes</button></form><form method='post' "
         "action='/meme/create'><button "
         "style='position: absolute; right: 0'>Create a new "
         "Meme</button></form></div><div class=column></div></div></body>"
         "<style>"
         "button{width: 30px; border-radius: 5px;background-color: navy;color: "
         "white;font-family: 'Sigmar One', cursive;width:50%;font-size: 1vw;}"
         ".template{background-color: white; margin: auto; width: 40vw;height: "
         "40vw;word-break: "
         "break-all;margin:1vw;font-family: "
         "'Oswald', sans-serif;text-transform: "
         "uppercase;background-image:url("
      << img_url
      << ");background-position: center; background-size:cover; color: "
         "white;font-size: "
         "2vw;-webkit-text-stroke: 1px black;position: "
         "relative;align-content: center;}"
         ".top-text{font-size: 130%;padding: 2% 0% 2% 0%; font-weight: "
         "bold; width: 100%;position: "
         "absolute;top:0;text-align: center;}"
         ".bottom-text{font-size: 130%;padding: 2% 0% 2% "
         "0%;font-weight:bold;width:100%;position: absolute;bottom: "
         "0;text-align: center;}.page-layout{width: 100%;height: 100%;display: "
         "grid;grid-template-rows:100%;grid-template-columns: 15% 70% 15%; "
         "position: "
         "relative;}.page-content{padding-left:5%;padding-right:5%;text-align: "
         "center;font-size: 1.5vw;}.column {display: "
         "grid;justify-content:center;align-content:center;text-align:center;"
         "position: relative}";

  enum template_style { white_font = 1, black_background = 2, black_font = 3 };
  int chosen_style = stoi(style);
  switch (chosen_style) {
  case white_font:
    break;
  case black_background:
    ss_resp << ".captions{background-color:black}";
    break;
  case black_font:
    ss_resp << ".captions{color:black; -webkit-text-stroke: 1px white}";
    break;
  default:
    return show_error_page(http::status::bad_request, "Invalid meme request");
  }
  ss_resp << "</style></body></html>";
  std::string response_content = ss_resp.str();

  http::response resp;
  resp.result(http::status::ok);
  resp.set(http::field::content_type, "text/html");
  resp.content_length(response_content.length());
  resp.body() = response_content;
  resp.prepare_payload();
  return resp;
}

http::response MemeHandler::browse_memes(const http::request &req) const {
  std::map<std::string, std::vector<std::string>> memes =
      parent_server_->get_memes();
  std::ostringstream ss;
  std::string style;
  enum template_style { white_font = 1, black_background = 2, black_font = 3 };
  int chosen_style;
  ss << "<!DOCTYPE html><html><head><link rel='preconnect' "
        "href='https://fonts.gstatic.com'>"
        "<link href"
        "='https://fonts.googleapis.com/"
        "css2?family=Odibee+Sans&family=Oswald:wght@700&family=Sigmar+One&"
        "display=swap' rel='stylesheet'></head>"
        "<style>"
        ".template{background-color: white; margin: auto; width: 300px;height: "
        "300px;word-break: "
        "break-all;margin:14px;font-family: "
        "'Oswald', sans-serif;text-transform: "
        "uppercase;"
        "background-position: center; background-size:cover; color: "
        "white;font-size: "
        "20px;-webkit-text-stroke: 1px black;position: "
        "relative;align-content: center;}"
        ".top-text{font-size: 20px;padding: 2% 0% 2% 0%; font-weight: "
        "bold; width: 100%;position: "
        "absolute;top:0;text-align: center;}"
        ".bottom-text{font-size: 20px;padding: 2% 0% 2% "
        "0%;font-weight:bold;width:100%;position: absolute;bottom: "
        "0;text-align: center;}button{padding-left: 2% width: 30px; "
        "border-radius: "
        "5px;background-color: navy;color: "
        "white;font-family: 'Sigmar One', cursive;width:50%;font-size: "
        "1vw;}</style>"
        "<body style=\"background-color:lightblue; font-family: 'Sigmar "
        "One', "
        "cursive;color: navy\"><h1 style='padding-left: 2%'>Browse "
        "Memes</h1><form method='post' "
        "action='/meme/create'><button "
        "style='position: "
        "absolute; left: 0'>Create a new "
        "Meme</button></form><br><br>"
        "<div style='display:flex; flex-wrap: wrap'>";
  for (auto &meme : memes) {
    chosen_style = stoi(meme.second[2]);
    switch (chosen_style) {
    case white_font:
      style = "color:white; -webkit-text-stroke: 1px black";
      break;
    case black_background:
      style = "background-color:black";
      break;
    case black_font:
      style = "color:black; -webkit-text-stroke: 1px white";
      break;
    default:
      return show_error_page(http::status::bad_request, "Invalid meme request");
    }
    ss << "<div "
          "class=template style=\"background-image:url('"
       << meme.first
       << "')\">"
          "<div class='top-text captions' style='"
       << style << "'>" << meme.second[0]
       << "</div>"
          "<div class='bottom-text captions' style='"
       << style << "'>" << meme.second[1] << "</div></div>";
  }
  ss << "</body></html>";
  std::string response_content = ss.str();

  http::response resp;
  resp.result(http::status::ok);
  resp.set(http::field::content_type, "text/html");
  resp.content_length(response_content.length());
  resp.body() = response_content;
  resp.prepare_payload();
  return resp;
}

http::response MemeHandler::create_meme(const http::request &req) const {
  std::string path = convertToAbsolutePath("./memes/meme_creation.html");
  std::ifstream ifs(path, std::ifstream::in);

  if (ifs.fail()) {

    return show_error_page(http::status::internal_server_error,
                           "error getting meme creation form");
  }
  std::stringstream ss;
  char c;
  while (ifs.get(c)) {
    ss << c;
  }
  std::string meme_form_content = ss.str();

  http::response resp;
  resp.result(http::status::ok);
  resp.set(http::field::content_type, "text/html");
  resp.content_length(meme_form_content.length());
  resp.body() = meme_form_content;
  resp.prepare_payload();
  return resp;
}

bool MemeHandler::parse(std::stringstream &ss_body,
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

bool MemeHandler::is_boundary(std::string boundary, std::string line) const {
  return line == "--" + boundary + "--\r" || line == "--" + boundary + "\r";
}