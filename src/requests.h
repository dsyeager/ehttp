#pragma once


enum request_state { CONNECTING = 0,
                     SSL_HANDSHAKE,
                     SEND_HEADERS,
                     RECEIVE_RESPONSE,
                     CLEANUP
                   };

class request
{
public:
    request(server *srv, uint32_t port, dsy::string_view url)
        :m_server(srv), m_port(port), m_url(url)
    { }

private:
    server *m_server = nullptr;
    uint32_t m_port = 0;
    dsy::string_view m_url = {0, 0};
    request_state m_state = CONNECTING;
    int m_socket = -1;
};

class requests
{
public:

    void add_request(server *srv, uint32_t port, dsy::string_view url)
    {
        request *req = new request(srv, port, url);
        m_requests.push_back(req);
    }

private:
    std::vector<request*> m_requests;
};
