#pragma once

#include <unistd.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <stack>
#include <tuple>
#include <thread>
#include <vector>

class thread_pool
{
public:
        thread_pool(size_t thread_cnt)
        {
                for (size_t i = 0; i < thread_cnt; i++)
                {
                        m_threads.emplace_back(std::thread([this](){this->thread_go();}));
                }
        }

        ~thread_pool()
        {
                {
                        std::unique_lock alock(m_mutex);
                        m_stop = true;
                        m_cond.notify_all();
                }
                
                for (auto &thrd : m_threads)
                {
                        thrd.join();
                }
        }

        void start(std::function<void(void*)> func, void* arg)
        {
                std::unique_lock alock(m_mutex);
                m_funcs.push(std::make_pair(func, arg));
                m_cond.notify_one();
                m_submits++;
        }

        void start_n(std::function<void(void*)> func, void* arg, size_t n)
        {
                for (size_t i = 0; i < n; i++)
                {
                        start(func, arg);
                }
        }

        uint32_t waiter_cnt() const { return m_waiters; }
        uint32_t active_cnt() const { return m_threads.size() - m_waiters; }

        bool wait_for(uint32_t milliseconds)
        {
                // do a busy check of m_waiters == m_threads.size
                uint64_t us_sleep = milliseconds / 10 * 1000;
                for (uint32_t i = 0; i < 10 && m_waiters != m_threads.size(); i++)
                {
                        usleep(us_sleep);
                }
                return m_waiters == m_threads.size();
        }

private:
        void thread_go()
        {
                while (!m_funcs.empty() || !m_stop)
                {
                        std::function<void(void*)> func;
                        void *arg = nullptr;

                        {
                                std::unique_lock alock(m_mutex);
                                if (!m_funcs.empty())
                                {
                                        std::tie(func, arg) = m_funcs.top();
                                        m_funcs.pop();
                                }
                                else if (m_funcs.empty() && m_stop)
                                {
                                        return;
                                }
                                else
                                {
                                        m_waiters++;
                                        m_cond.wait(alock);
                                        m_waiters--;
                                        if (m_funcs.empty() && m_stop)
                                        {
                                                return;
                                        }
                                        else if (m_funcs.empty())
                                        {
                                                // spurious wakeup
                                                continue;
                                        }
                                        std::tie(func, arg) = m_funcs.top();
                                        m_funcs.pop();
                                }
                        }
                        func(arg);
                }                
        }

private:
        std::vector<std::thread> m_threads;
        std::stack<std::pair<std::function<void(void*)>, void*>> m_funcs;
        std::mutex m_mutex;
        std::condition_variable m_cond;
        std::atomic<uint32_t> m_waiters{0};
        volatile bool m_stop = false;
        uint64_t m_submits = 0;
};
