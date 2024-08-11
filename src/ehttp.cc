#include "get_nanoseconds.h"
#include "requests.h"
#include "servers.h"
#include "thread_pool.h"
#include "times.h"

#include <poll.h>
#include <stdio.h>

#include <utility>
#include <iomanip>
#include <iostream>
#include <string_view>
#include <vector>

using namespace std;

servers s_servers;
std::string s_url("/index.html");

std::mutex s_mutex;

bool check_stdin()
{
    for (uint32_t i = 0; i < 10; i++)
    {
        struct pollfd fds;
        int ret;
        fds.fd = 0; // this is STDIN
        fds.events = POLLIN;
        ret = poll(&fds, 1, 0);
        if(ret == 1)
            return true;
        usleep(100000);
    }
    return false;
}

void process_reqs(void *) // ingnore the arg
{
    auto tid = std::this_thread::get_id();
    requests reqs;

    while (true)
    {
		if (reqs.get_req_cnt() >= 10)
		{
			reqs.process();
			continue;
		}

        const server *srv = s_servers.get_server();

        if (!srv)
        {
            break;
        }
        
        {
            double us = double(srv->get_dns_ns()) / 1000;
            std::unique_lock alock(s_mutex);
            cout << "process_reqs, srv: " << srv->name() << ", tid: " << tid
                 << ", dns us: " << us
                 << endl;
            srv->print_ips();
        }

        if (!reqs.add_request(srv, 80, s_url))
		{
			std::cerr << "add_request failed" << std::endl;
		}
        //usleep(10);
    }

	if (reqs.get_req_cnt())
	{
		reqs.process();
	}

    cout << "thread exiting" << endl;
}

int main(int argc, char** argv)
{
    std::cout << std::fixed << std::setprecision(3);
    uint64_t start = get_nanoseconds();
    dsy::times::add("start"sv);
    const uint32_t thread_cnt = 1;
    thread_pool tpool(thread_cnt);
    tpool.start_n(process_reqs, nullptr, thread_cnt);
    uint16_t port = 10091;

    dsy::times::add("threads started"sv);
    string sbuff;
    bool use_stdin = false;
    s_servers.unpersist_servers("dns.cached");

    dsy::times::add("dns uncached"sv);

    for (int i = 1; i < argc; i++)
    {
        auto [key, val] = split(argv[i], '=');
        if (key == "--port"sv)
            port = atoi(val.begin());
    }

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
    }

    if (use_stdin || (s_servers.empty() && check_stdin()))
    {
        char *line = nullptr;
        size_t len = 0;

        while (cin)
        {
            //getline(cin, buff);
            ssize_t res = getline(&line, &len, stdin);
            if (res <= 0)
                break;
            sbuff.append(line, res);
            //sbuff += '\n';
        }
        free(line);
        s_servers.add_servers(sbuff, port);
    }

    if (s_servers.empty()) 
    {
        cerr << "No servers found" << endl;
        exit(1);
    }

    dsy::times::add("servers read"sv);

    //s_servers.print_servers();

    s_servers.resolve_addrs();

    cout << "servers resolved, ready to start processing requests" << endl;

    // ready to start the threads processing the requests








    while (tpool.active_cnt())
    {
        cout << "active threads: " << tpool.active_cnt() << ", server waiters: " << s_servers.waiter_cnt() << endl;
        tpool.wait_for(1000);
    }

    // all requests are processed, do some cleanup

    dsy::times::add("servers processed"sv);
    s_servers.persist_servers("dns.cached", 5);
    dsy::times::add("dns cached"sv);
    uint64_t end = get_nanoseconds();
    dsy::times::show();
    cout << "total milliseconds: " << (double(end - start) / 1000000) << endl;
}
