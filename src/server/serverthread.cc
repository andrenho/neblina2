#include "serverthread.hh"

void ServerThread::action(SOCKET&& fd) {}
void ServerThread::add_session(std::unique_ptr<Session> session) {}
void ServerThread::remove_socket(SOCKET fd) {}


