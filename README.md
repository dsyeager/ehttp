# ehttp
Event driven http client for requesting http from one or many remote hosts

Event driven: 
    epoll is used for working with events.
Multi-threaded:
    A single thread can use at most 100% of one CPU.
    By starting N threads we can use at most 100% of N CPU's.
    This becomes more necessary when dealing with SSL, the encryption can use a lot of CPU.
    Each thread creates it's own requests object which handles epoll processing.
    The main thread server names to a central queue. The threads pop server names off the queue and start request objects as they have capacity.

HTTP
    It is initially HTTP 1.1 and handles a single URL per run. 

