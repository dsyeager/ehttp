#pragma once

#include "http_response.h"
#include "log.h"
#include "servers.h"

#include <sys/epoll.h>
#include <unistd.h>


/**
  Initial design is to support a single http 1.1 request per server.

  Goal should be to split the http handling out into classes to allow the use of http 1.1 or http 2.0 or maybe http 3.0 (quic) eventually

  The requests/request class would handle the epoll event loop/handling and the http_response_<type> class would do the reading/parsing.

  Could also set it up to allow a set of urls to be fetched. Pipeline for http1.1 and parralel for http 2.0
  */

enum request_state { CONNECTING = 0,
                     SSL_HANDSHAKE,
                     SEND_HEADERS,
                     RECEIVE_RESPONSE,
                     CLEANUP,
                     DONE,
                     FAILED
                   };
std::string_view s_request_states[] { "CONNECTING", "SSL_HANDSHAKE", "SEND_HEADERS", "RECEIVE_RESPONSE", "CLEANUP", "DONE", "FAILED" };

dsy::string_view event_to_str(auto event, std::string &str)
{
    if (event & EPOLLIN)
        str << "EPOLLIN" << '|';

    if (event & EPOLLOUT)
        str << "EPOLLOUT" << '|';

    if (event & EPOLLRDHUP )
        str << "EPOLLRDHUP" << '|';

    if (event & EPOLLPRI)
        str << "EPOLLPRI" << '|';

    if (event & EPOLLERR)
        str << "EPOLLERR" << '|';

    if (event & EPOLLHUP)
        str << "EPOLLHUP" << '|';

    return dsy::string_view(str).rtrim('|');
}

class request
{
public:
    request(const dsy::server *srv,
            uint32_t port,
            dsy::string_view url,
            bool ssl,
            void (*read_headers_cb)(const http_response *resp),
            void (*read_body_cb)(std::string_view data, const http_response *resp))
        :m_server(srv),
         m_port(port),
         m_url(url),
         m_ssl(ssl),
         m_response(srv->name()),
         m_read_headers_cb(read_headers_cb),
         m_read_body_cb(read_body_cb)
    {
        m_socket = srv->non_blocking_connect();
	    TRACE(1) << "non_blocking_connect returned: " << m_socket << ENDL;

        m_event.data.fd = m_socket;
        m_event.data.ptr = this;
        m_event.events = EPOLLOUT | EPOLLET;
    }

    void process(auto events) 
    {
        TRACE(1) << "request::process, state: " << s_request_states[m_state] << ", event: " << event_to_str(events, m_buff.erase()) << ENDL;

        switch (m_state) {
        case CONNECTING:
            if (events & EPOLLOUT)
            {
                if (m_ssl)
                {
                    m_state == SSL_HANDSHAKE;
//                    this->ssl_handshake();
                } 
                else
                {
                    set_state(SEND_HEADERS);
                    this->send_headers();
                }
            }
            break;
        case SSL_HANDSHAKE:
            break;
        case SEND_HEADERS:
            break;
        case RECEIVE_RESPONSE:
            receive_response();
            break;
        case CLEANUP:
            break;
        case DONE:
            break;
        }
    }

    void send_headers()
    {
        std::string req;
        req << "GET " << m_url << " HTTP/1.1\r\n";
        req << "Host: " << m_server->name() << "\r\n";
        req << "User-Agent: ehttp\r\n";
//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
//Accept-Encoding: gzip, deflate, br
        req << "Connection: close\r\n\r\n"; 

        ssize_t bw = write(m_socket, req.data(), req.size());

        TRACE(1) << "wrote " << bw << " out of " << req.size() << " bytes" << ENDL;
        TRACE(1) << '\n' << req << ENDL;

        if (bw != req.size())
        {
            set_state(FAILED);
            this->close();
            return;
        }
// TODO make room for a SEND_BODY state
        set_state(RECEIVE_RESPONSE);
        m_event.events = EPOLLIN | EPOLLET;
TRACE(1) << "reset events to EPOLLIN | EPOLLET" << ENDL;
// need to change the event we are polling on
    }

    void receive_response()
    {
        TRACE(1) << ENDL;

        while (true)
        {
            ssize_t br = m_response.read(m_socket, m_read_headers_cb, m_read_body_cb); 
            if (br == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    return;
                }
                else
                {
                    set_state(FAILED);
                    //this->close();
                    return;
                }
            }
            else if (br == 0)    
            {
                if (m_response.body_done())
                {
                    TRACE(1) << "response complete, need to change state" << ENDL;
                }
                return;
            }
            else
            {
                if (m_response.body_done())
                {
                    set_state(DONE);
                    //this->close();
                }
TRACE(1) << "http_response::read returned: " << br << ENDL;
            }
        } 
    }

    void set_state(request_state state)
    {
        m_state = state;
        TRACE(1) << "server: " << m_server->name() << ", set_state: " << s_request_states[m_state] << ENDL; 
    }

    bool is_done() const { return m_state > RECEIVE_RESPONSE; } 

    void close()
    {
        if (m_socket != -1)
            ::close(m_socket);
        m_socket = -1;
    }

    std::string_view srv_name() const { return m_server->name(); }

    int get_fd() const { return m_socket; }
    uint16_t port() const { return m_port; }
    request_state state() const { return m_state; }
    epoll_event* ep_event() { return &m_event; } 

private:
    void parse_headers()
    {
        dsy::string_view data(m_headers);
        size_t pos = data.find("\r\n\r\n");
        if (pos == dsy::string_view::npos)
        {
            TRACE(1) << "double crlf not found" << ENDL;
            return;
        }
        dsy::string_view hdrs = data.substr(0, pos);
        TRACE(1) << '\n' << hdrs << ENDL;
    }

private:
    const dsy::server *m_server = nullptr;
    uint16_t m_port = 0;
    bool m_ssl = false;
    dsy::string_view m_url = {0, 0};
    request_state m_state = CONNECTING;
    int m_socket = -1;
    epoll_event m_event;
    std::string m_buff;
    std::string m_headers;
    http_response m_response;

    void (*m_read_headers_cb)(const http_response *resp) = nullptr;
    void (*m_read_body_cb)(std::string_view data, const http_response *resp) = nullptr;
};

/**
    EPOLL_CTL_ADD EPOLL_CTL_MOD EPOLL_CTL_DEL
  */
class requests
{
public:
    requests()
        :m_epoll_fd(epoll_create(1)), m_events(new epoll_event[m_max_events])
    {
        if (m_epoll_fd == -1)
        {
            ERROR << "requests(), epoll_create failed: " << strerror(errno) << ENDL;
        }
    }

    // my historic use is multiple servers all using the same url and header/body cb's
    // but just like there could be a need for multiple urls
    // there could be a need for multiple cb's
    // so lets pass in the header/body cb's here and save those in the request object

    bool add_request(const dsy::server *srv,
                     dsy::string_view url,
                     bool ssl,
                     void (*read_headers_cb)(const http_response *resp),
                     void (*read_body_cb)(std::string_view data, const http_response *resp)
                     )
    {
        if (m_epoll_fd == -1)
            return false;
        request *req = new request(srv, srv->port(), url, ssl, read_headers_cb, read_body_cb);
        int fd = req->get_fd();
        TRACE(1) << "fd: " << fd << ENDL;

        if (fd != -1)
        {
            // m_requests.push_back(req);
            // this is where we do the initial add of the fd into the epoll cltn looking for a writable condition
            int res = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, req->ep_event());  
            if (res == -1)
            {
                ERROR << "epoll_ctl(add) failed: " << strerror(errno) << ENDL;
                delete req;
            }
            else
            {
                TRACE(1) << "added server fd (" << fd << ") to epoll fd (" << m_epoll_fd << ')' << ENDL;
                m_epoll_req_cnt++;
            }
            return true;
        }
        return false;
    }

    size_t process()
    {
        TRACE(2) << ENDL;
        // will need ability to loop until a specified timeout
        // which means configuring the fd's to have a timeout or ???
        return process_event_loop();
    }

    size_t get_req_cnt() const { return m_epoll_req_cnt; }

private:
    int process_event_loop()
    {
        TRACE(2) << "calling epoll_wait" << ENDL;
        int n = epoll_wait(m_epoll_fd, m_events, m_max_events, 100);

        TRACE(2) << "epoll events n: " << n << ENDL;

        for (int i = 0; i < n; i++)
        {
            request *req = static_cast<request*>(m_events[i].data.ptr);
            auto events = m_events[i].events;
            if ((events & EPOLLERR) ||
                (events & EPOLLHUP))
            {
                // TODO stream out the full server info for the failed entry
                ERROR << "epoll error, fd: " << m_events[i].data.fd 
                      << ", srv: " << req->srv_name() 
                      << ", fd: " << req->get_fd() << ENDL;

                int res = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, req->get_fd(), nullptr); 
                if (res == -1)
                {
                    ERROR << "epoll_ctl(del) failed: " << strerror(errno) << ENDL;
                }
                m_epoll_req_cnt--;

                req->set_state(FAILED);
                req->close();
                continue;
            }
            else if ((events & EPOLLIN) || (events & EPOLLOUT))
            {
                req->process(events);
                if (req->is_done())
                {
                    int res = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, req->get_fd(), nullptr); 
                    if (res == -1)
                    {
                        ERROR << "epoll_ctl(del) failed: " << strerror(errno) << ENDL;
                    }
                    m_epoll_req_cnt--;
                    req->close();
                    delete req;
                }
                else
                {
                    int res = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, req->get_fd(), req->ep_event()); 
                    if (res == -1)
                    {
                        ERROR << "epoll_ctl(mod) failed: " << strerror(errno) << ENDL;
                    }
                }
            }
            else
            {
                ERROR << "process_event_loop, unhandled events" 
                      << ", srv: " << req->srv_name() 
                      << ", fd: " << req->get_fd() << ENDL;
            }
        }
        return n;
    }

private:
    //std::vector<request*> m_requests;
    uint32_t m_epoll_req_cnt = 0;
    int m_epoll_fd = 0;
    uint16_t m_max_events = 500;
    epoll_event *m_events = nullptr;
};
