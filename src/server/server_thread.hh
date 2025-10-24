#ifndef NEBLINA_SERVER_THREAD_HH
#define NEBLINA_SERVER_THREAD_HH

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <thread>

#include "util/socket.hh"
#include "protocol/session.hh"

class ServerThread {
public:
    explicit ServerThread(class Server* server);

    ~ServerThread();

    void add_session(std::unique_ptr<Session> session);
    void remove_socket(SOCKET fd);

    void new_data_ready(SOCKET fd);

private:
    class Server* server_;
    std::unordered_map<SOCKET, std::unique_ptr<Session>> sessions_;
    std::atomic<bool> running_ = true;
    std::thread thread_;
    std::mutex sessions_mutex_;
    std::mutex queue_mutex_;
    std::condition_variable cond_;

    std::deque<SOCKET> queue_;
    std::unordered_set<SOCKET> queue_unique_;

    SOCKET pop_from_queue();
    void process_socket(SOCKET fd);
};


#endif //NEBLINA_SERVER_THREAD_HH
