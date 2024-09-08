#include "get_nanoseconds.h"
#include "log.h"
#include "requests.h"
#include "servers.h"
#include "stdin_helpers.h"
#include "thread_pool.h"
#include "times.h"

#include <poll.h>
#include <stdio.h>

#include <utility>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string_view>
#include <vector>

using dsy::string_view, dsy::logs;
using std::string, std::vector, std::cin, std::cout, std::endl;
using namespace std::literals;

dsy::servers s_servers;
string s_url("/index.html");
bool s_ssl(false);

std::mutex s_mutex;

void read_headers_cb(const http_response *resp)
{
    TRACE(1) << "read_headers_cb, server: " << resp->server() << ", header cnt: " << resp->headers()->size() << ENDL;
}

void read_body_cb(std::string_view data, const http_response *resp)
{
    TRACE(1) << "read_body_cb, server: " << resp->server() << ", bytes: " << data.size() << ENDL;
    dsy::string_view body(data);
    while (body.size())
    {
        dsy::string_view line = body.split('\n');
        line.rtrim('\r');
        std::cout << resp->server() << ' ' << line << std::endl; 
    }
}

void process_reqs(void *) // ingnore the arg
{
    auto tid = std::this_thread::get_id();
    requests reqs;
    string buff;

    while (true)
    {
		if (reqs.get_req_cnt() >= 100) // should be a configurable max
		{
			reqs.process();
			continue;
		}

        const dsy::server *srv = s_servers.get_server();

        if (!srv)
        {
            TRACE(1) << "breaking from while(srv) loop" << ENDL;
            break;
        }
        
        TRACE(1) << "srv: " << srv->name() << ", tid: " << tid << ", dns us: " << (double(srv->get_dns_ns()) / 1000) << ENDL;
        buff.clear();
        TRACE(1) << srv->to_str(buff) << ENDL;

        if (!reqs.add_request(srv, s_url, s_ssl, read_headers_cb, read_body_cb))
		{
			ERROR << "add_request failed" << ENDL;
		}
    }

	while (reqs.get_req_cnt())
	{
        TRACE(2) << "req cnt: " << reqs.get_req_cnt() << ENDL;
		reqs.process();
	}

    TRACE(1) << "thread exiting" << ENDL;
}

int main(int argc, char** argv)
{
    std::cout << std::fixed << std::setprecision(2);

    uint64_t start = get_nanoseconds();
    dsy::times::add("start"sv);
    bool show_times = false;
    const uint32_t thread_cnt = 1;
    uint16_t port = 10091;

    dsy::times::add("threads started"sv);

    string sbuff;
    bool use_stdin = false;

    for (int i = 1; i < argc; i++)
    {
        auto [key, val] = split(argv[i], '=');
        if (key == "--port"sv)
            port = atoi(val.begin());
        else if (key == "--verbose"sv || key == "-v"sv)
            logs::verbose++;
        else if (key == "--show-times"sv || key == "--times"sv)
            show_times = true;
    }

    thread_pool tpool(thread_cnt);
    tpool.start_n(process_reqs, nullptr, thread_cnt);

    s_servers.unpersist_servers("dns.cached");

    dsy::times::add("dns uncached"sv);

    for (int i = 1; i < argc; i++)
    {
        auto [key, val] = split(argv[i], '=');
        if (key == "--servers"sv)
        {
            if (val == "stdin"sv)
                use_stdin = true;
            else
                s_servers.add_servers(val, port);
        }
        else if (key == "--ssl"sv)
        {
            s_ssl = true;
        }
    }

    dsy::times::add("cmd line args processed"sv);

    if (use_stdin || (s_servers.empty() && stdin_has_data()))
    {
        read_stdin(sbuff);
        if (sbuff.size())
            TRACE(1) << "cin servers: " << sbuff << ENDL;
        s_servers.add_servers(sbuff, port);
    }

    if (s_servers.empty()) 
    {
        ERROR << "No servers found" << ENDL;
        exit(1);
    }

    dsy::times::add("servers read"sv);

    //s_servers.print_servers();

    s_servers.resolve_addrs();

    TRACE(1) << "servers resolved, ready to start processing requests" << ENDL;

    // ready to start the threads processing the requests

    while (tpool.active_cnt())
    {
        TRACE(1) << "active threads: " << tpool.active_cnt() << ", server waiters: " << s_servers.waiter_cnt() << ENDL;
        tpool.wait_for(1000);
    }

    // all requests are processed, do some cleanup

    dsy::times::add("servers processed"sv);
    s_servers.persist_servers("dns.cached", 5);
    dsy::times::add("dns cached"sv);
    uint64_t end = get_nanoseconds();
    if (show_times)
    {
        dsy::times::show();
    }
    TRACE(1) << "total milliseconds: " << (double(end - start) / 1000000) << ENDL;
}
