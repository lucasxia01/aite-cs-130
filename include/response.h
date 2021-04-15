#include <string>

namespace http {
struct response {
  enum status_type { OK = 200, BAD_REQUEST = 400 } status;
};
} // namespace http