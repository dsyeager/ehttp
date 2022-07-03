#pragma once



class event
{
    event()
	: m_ep_event()
    {
        m_ep_event.data.ptr = this;
        m_ep_event.events = 0;
    }

    void set_event(event_type type, std::function<void(void*)> cb)
    {
        if (m_is_in_queue) return;

        m_type = type;
        m_cb = cb;
 
        m_ep_event.data.ptr = this;
        m_ep_event.events = 0;

        switch (type) {
        case WRITE:
            m_ep_event.events |= EPOLLOUT;
            break;
        case READ:
            m_ep_event.events |= EPOLLIN;
            break;
        case READ_WRITE:
            m_ep_event.events |= EPOLLOUT;
            m_ep_event.events |= EPOLLIN;
            break;
        }
    }


    epoll_event* get_ep_event() { return &m_ep_event; }

    void process()
    {
        if (-1 == m_fd)
        {
            std::cerr << "event::process called but m_fd is -1" << std::endl;
        } 
        else
        {
            m_cb(this);
        }
    }

    int  fd() const { return m_fd; }
    void set_is_in_queue(bool val) { m_is_in_queue = val; }
    void set_failed(dsy::string_view err)
    {
        int sock_err = 0;
        socklen_t elen = sizeof(sock_err);
        std::cerr << "event_set_failed, " << err 
        
        if (setsockopt(m_fd, SOL_SOCKET, SO_ERROR, (void *)&sock_err, &elen) == 0)
        {
            std::cerr << " (" << strerror(sock_err) << ')'; 
        }
        std::cerr << std::endl;
    }

    bool is_in_queue() const { return m_is_in_queue; }

protected:
    int m_fd = -1;
    uint64_t m_expires = UINT64_MAX;
    std::function<void(void*)> m_cb;
    event_type m_etype = WRITE;
    bool m_is_in_queue = false;
    epoll_event m_ep_event;
};


class events
{
public:
    events();
    ~events();

    bool add_event(event *p_event);
    void remove_event(event *p_event);
    int32_t dispatch(uint64_s wait_us);
private:
    int m_ep_fd = -1;
};
