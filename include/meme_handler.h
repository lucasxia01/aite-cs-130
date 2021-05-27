#ifndef NEME_HANDLER_H
#define NEME_HANDLER_H

#include "request_handler.h"
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <regex>

/**
 * Handler that's used for testing multithreading
 */
class MemeGenHandler : public RequestHandler {
public:
	MemeGenHandler() {}
	http::response handle_request(const http::request &req) const;
};

#endif // NEME_HANDLER_H