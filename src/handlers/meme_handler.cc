#include "meme_handler.h"

http::response
MemeGenHandler::handle_request(const http::request &req) const {
	std::string body = req.body();
	if (
	    req.method() != http::verb::post ||
	    req.find(http::field::content_type) == req.end()) {
		return show_error_page(http::status::bad_request, "Invalid upload request");
	}
	std::string content_type = req.at(http::field::content_type).to_string();

	std::stringstream ss;
	ss << req;
	std::string req_s = ss.str();

	if (content_type.find("multipart/form-data") == std::string::npos) {
		return show_error_page(http::status::unsupported_media_type, "Invalid upload content type");
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

	LOG_DEBUG << "PARSED BOUNDARY: " << boundary;

	std::stringstream ss_body;
	ss_body << req.body();
	std::string ss_body_s = ss_body.str();

	std::string curr_line;
	std::size_t boundary_count = 0;

	// Validate number of boundaries in the body is 2, since there should only be one attached file from the form
	while (std::getline(ss_body, curr_line)) {
		if (curr_line.find(boundary) != std::string::npos) {
			boundary_count++;
		}
	}
	if (boundary_count != 2) {
		return show_error_page(http::status::bad_request, "Too many form parts");
	}

	// Parse part inside boundaries
	std::string body_in_boundaries;
	std::smatch match_body_in_boundaries;
	std::string r_temp = "--" + boundary + "([^]*)" + boundary + "--";
	std::regex r_body_in_boundaries(r_temp);

	if (std::regex_search(ss_body_s, match_body_in_boundaries, r_body_in_boundaries)) {
		body_in_boundaries = match_body_in_boundaries[1];
		boost::trim(body_in_boundaries);
	} else {
		return show_error_page(http::status::bad_request, "Invalid multipart part");
	}

	std::string b_header;
	std::string b_body;
	std::smatch match_b;
	std::regex r_b("^([^]*)\r\n\r\n([^]*)$");
	if (std::regex_search(body_in_boundaries, match_b, r_b) && match_b.size() == 3) {
		b_header = match_b[1];
		b_body = match_b[2];
		boost::trim(b_header);
		boost::trim(b_body);
	} else {
		return show_error_page(http::status::bad_request, "Invalid part");
	}

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

	curr_content_type = curr_content_type.substr(curr_content_type.find("image/") + 6); // image/jpeg -> jpeg

	std::filesystem::path path{"./memes"};
	boost::uuids::uuid u;
	path /= boost::uuids::to_string(u) + "." + curr_content_type;
	std::filesystem::create_directories(path.parent_path());
	std::ofstream ofs(path);
	ofs << b_body;
	ofs.close();

	http::response resp;
	resp.result(http::status::ok);
	resp.set(http::field::content_type, "text/plain");
	resp.body() = "pls make some memes\n";
	resp.prepare_payload();
	return resp;
}