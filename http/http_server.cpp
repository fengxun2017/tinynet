
handle_message(TcpConnPtr conn, const uint8_t *data, size_t size) {
    std::string raw_request(reinterpret_cast<const char*>(data), size);
    HttpRequest request;
    if (!request.parse(raw_request)) {
        std::cerr << "Failed to parse HTTP request" << std::endl;
        return;
    }

    HttpResponse response;
    if (request.method == HttpRequest::GET) {
        response.set_body("Hello, world!");
    } else {
        response.set_status(400, "Bad Request");
        response.set_body("Unsupported method");
    }

    std::string raw_response = response.to_string();
    conn->send(reinterpret_cast<const uint8_t*>(raw_response.data()), raw_response.size());

    if (_on_message_cb) {
        _on_message_cb(conn, data, size);
    }