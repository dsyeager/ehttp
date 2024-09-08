#pragma once

#include "log.h"
#include "string_helpers.h"
#include "string_view.h"

#include <unistd.h>

class http_response
{
public:
    http_response(std::string_view srv)
        :m_server(srv)
    {

    }

    int64_t read(int fd,
                 void (*read_headers_cb)(const http_response *resp),
                 void (*read_body_cb)(std::string_view data, const http_response *resp))
    {
// read_headers will likely read some of the body
        if (m_headers.empty())
            return read_headers(fd, read_headers_cb, read_body_cb);

        ssize_t total_read = 0;
        char buff[10000];
        while (true)
        {
            auto [ret, br] = read_socket(fd, buff, sizeof(buff) - 1, total_read);
            if (!ret)
            {
                return br;
            }

            total_read += br;
            m_content_read += br;
            read_body_cb(dsy::string_view(buff, br), this);
            TRACE(1) << "body bytes read: " << br << ", total: " << total_read << ", content_read: " << m_content_read << ", content-length: " << m_content_length << ENDL;

            if (m_content_length > -1 && m_content_read >= m_content_length) // done reading response content
            {
                m_body_done = true;
                return total_read;
            }
        }
    }

    int64_t content_length() const { return m_content_length; }
    bool headers_parsed() const { return m_headers.size(); }
    bool body_done() const { return m_body_done; }
    bool failed() const { return m_failed; }
    std::string_view server() const { return m_server; }
    const std::vector<dsy::string_view>* headers() const { return &m_headers; };
private:
    std::pair<bool, ssize_t> read_socket(int fd, char *buff, size_t cnt, ssize_t total_read)
    {
        // return false for caller to return early
        // return true for caller to continue processing
        ssize_t br = ::read(fd, buff, cnt);
        if (br == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return std::make_pair(false, total_read);
            }
            else
            {
                m_failed = true;
                return std::make_pair(false, br);
            }
        }
        else if (br == 0)
        {
            return std::make_pair(false, total_read);
        }
        return std::make_pair(true, br);
    }

    int64_t read_headers(int fd,
                         void (*read_headers_cb)(const http_response *resp),
                         void (*read_body_cb)(std::string_view data, const http_response *resp))
    {
        TRACE(2) << ENDL;
        ssize_t total_read = 0;
        char buff[10000];
        while (true)
        {
            auto [ret, br] = read_socket(fd, buff, sizeof(buff) - 1, total_read);
            if (!ret)
            {
                return br;
            }

            total_read += br;
            m_header_buffer.append(buff, br);
            if (parse_headers(read_headers_cb, read_body_cb))
            {
                return total_read;
            }

            TRACE(1) << "total_read: " << total_read << ENDL;
        }
    }
    
    bool parse_headers(void (*read_headers_cb)(const http_response *resp),
                       void (*read_body_cb)(std::string_view data, const http_response *resp))
    {
        using namespace std::literals;
        if (m_headers.size())
            return true; // already parsed

        static const dsy::string_view dbcrlf("\r\n\r\n"sv);
        static const dsy::string_view crlf("\r\n"sv);

        size_t pos = m_header_buffer.find(dbcrlf);

        if (pos == std::string::npos)
            return false;

        // RFC 7230 deprecates line folding (multi line headers)

        dsy::string_view bytes(m_header_buffer);
        dsy::string_view hdrs = bytes.substr(0, pos);
        dsy::string_view body = bytes.substr(pos + 4);

        hdrs.split(crlf, m_headers);

        for (auto hdr : m_headers)
        {
            auto [key, val] = split(hdr, ':');
            key.rtrim(' '); // should trim on any type of whitespace
            val.ltrim(' ');
            // don't filter headers here, just grab the values needed.
            if (key.equals_ci("content-length"sv))
            {
                val.aton(m_content_length); 
                TRACE(1) << "content-length: " << m_content_length << ENDL;
            }
            else if (key.equals_ci("transfer-encoding"sv))
            {
                if (val.equals_ci("chunked"sv))
                    m_CTE = true;
                else
                    ERROR << "unhandled transfer-encoding value: " << val << ENDL;
            }
        }
        read_headers_cb(this);
        m_content_read += body.size();
        if (body.size())
            read_body_cb(body, this);

        m_body_done = body.size() >= m_content_length;;

        TRACE(1) << "srv: " << m_server
                 << ", content-length: " << m_content_length
                 << ", body_bytes: " << body.size() << ENDL;
        return true;
    }
private:
    int64_t m_content_length = -1;
    int64_t m_content_read = 0;
    bool m_CTE = false;
    bool m_body_done = false;
    bool m_failed = false;
    std::string m_server;
    std::string m_header_buffer;
    std::vector<dsy::string_view> m_headers;
};
