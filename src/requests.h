#pragma once

#include "servers.h"


enum request_state { CONNECTING = 0,
                     SSL_HANDSHAKE,
                     SEND_HEADERS,
                     RECEIVE_RESPONSE,
                     CLEANUP
                   };

class request
{
public:
    request(const server *srv, uint32_t port, dsy::string_view url)
        :m_server(srv), m_port(port), m_url(url)
    {
        m_socket = srv->non_blocking_connect();
	std::cerr << "non_blocking_connect returned: " << m_socket << std::endl;
    }


    int get_fd() const { return m_socket; }
private:
    const server *m_server = nullptr;
    uint32_t m_port = 0;
    dsy::string_view m_url = {0, 0};
    request_state m_state = CONNECTING;
    int m_socket = -1;
};

class requests
{
public:
    requests()
    {
    }

    bool add_request(const server *srv, uint32_t port, dsy::string_view url)
    {
        request *req = new request(srv, port, url);
        if (req->get_fd() != -1)
        {
            m_requests.push_back(req);
            // this is where we do the initial add of the fd into the epoll cltn looking for a writable condition
            return true;
        }
        return false;
    }

    size_t process()
    {

        return m_requests.size();
    }

    size_t get_req_cnt() const { return m_requests.size(); }

private:
    std::vector<request*> m_requests;
};
