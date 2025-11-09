#include "neblina.hh"

#include <cstdio>
#include <cstdlib>

#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include <csignal>

#ifndef _WIN32
#  include <unistd.h>
#  include <sys/wait.h>
#else
#  include <windows.h>
#endif

#include "ya_getopt.h"

//------------------------------------------------------
// PARSE OPTIONS
//------------------------------------------------------

enum class ClientType { None, TCP, SSL };

struct Config {
    ClientType client_type = ClientType::None;
    size_t n_attempts = 1;
    size_t n_threads  = 1;
    bool   run_server = false;
};

static void show_help()
{
    fprintf(stderr, "Usage: neblina-load-test [--tcp|--ssl] [--attempts N] [--threads N] [--run-server]\n");
    fprintf(stderr, "  -t, --tcp          Use TCP client\n");
    fprintf(stderr, "  -s, --ssl          Use SSL client\n");
    fprintf(stderr, "  -a, --attempts     Number of attempts (default 1)\n");
    fprintf(stderr, "  -n, --threads      Number of threads  (default 1)\n");
    fprintf(stderr, "  -r, --run-server   Execute server\n");
    exit(EXIT_FAILURE);
}

Config parse_args(int argc, char** argv)
{
    Config cfg {};

    const option long_opts[] = {
            {"tcp",        no_argument,       nullptr, 't'},
            {"ssl",        no_argument,       nullptr, 's'},
            {"attempts",   required_argument, nullptr, 'a'},
            {"threads",    required_argument, nullptr, 'n'},
            {"run-server", required_argument, nullptr, 'r'},
            {"help",       no_argument,       nullptr, 'h'},
            {nullptr,      0,                 nullptr, 0}
    };

    int opt;
    int opt_idx;
    while (true) {
        opt = getopt_long(argc, argv, "tsa:n:rh", long_opts, &opt_idx);
        if (opt == -1)
            break;

        switch (opt) {
            case 't':
                if (cfg.client_type != ClientType::None) {
                    fprintf(stderr, "Only one of --tcp or --ssl may be specified\n");
                    exit(EXIT_FAILURE);
                }
                cfg.client_type = ClientType::TCP;
                break;

            case 's':
                if (cfg.client_type != ClientType::None) {
                    fprintf(stderr, "Only one of --tcp or --ssl may be specified\n");
                    exit(EXIT_FAILURE);
                }
                cfg.client_type = ClientType::SSL;
                break;

            case 'a':
                cfg.n_attempts = strtoull(optarg, nullptr, 10);
                break;

            case 'n':
                cfg.n_threads = strtoull(optarg, nullptr, 10);
                break;

            case 'r':
                cfg.run_server = true;
                break;

            case '?':
                break;

            case 'h':
            default:
                show_help();
                std::exit(opt == 'h' ? EXIT_SUCCESS : EXIT_FAILURE);
        }
    }

    if (cfg.client_type == ClientType::None) {
        std::cerr << "Error: one of --tcp or --ssl is required\n";
        std::exit(EXIT_FAILURE);
    }

    return cfg;
}

//---------------------------
// RUN SERVER
//---------------------------

#ifndef _WIN32

pid_t run_server(ClientType type)
{
    pid_t pid = fork();
    if (pid == 0) {  // server
        execlp("./neblina-load-test-server", "./neblina-load-test-server", type == ClientType::TCP ? "-t" : "-s", nullptr);
        perror("execlp");
        _exit(127);
    } else {
        std::this_thread::sleep_for(300ms);
    }
    return pid;
}

void kill_server(pid_t pid)
{
    kill(pid, SIGTERM);

    int status;
    waitpid(pid, &status, 0);
}

#else

using pid_t = PROCESS_INFORMATION;

pid_t run_server(ClientType type)
{
    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    std::string cmd = "./neblina-load-test-server ";
    cmd += (type == ClientType::TCP) ? "-t" : "-s";

    if (!CreateProcessA(
            NULL,
            cmd.data(),          // command line
            NULL, NULL,
            FALSE,
            0,
            NULL, NULL,
            &si,
            &pi))
    {
        throw std::runtime_error("Failed to start server");
    }

    // Server started; give it time to initialize.
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    return pi; // caller must close handles
}

void kill_server(pid_t h)
{
    // Try to send a graceful CTRL-C first (only works if server is a console process
    // in same console group; if not, fallback to TerminateProcess).
    // Equivalent to SIGTERM intent.
    BOOL sent = GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, h.dwProcessId);

    // Give process a chance to exit gracefully
    if (sent) {
        if (WaitForSingleObject(h.hProcess, 3000) == WAIT_OBJECT_0)
            goto cleanup;
    }

    // Force kill (SIGKILL equivalent)
    TerminateProcess(h.hProcess, 1);

    WaitForSingleObject(h.hProcess, INFINITE);

cleanup:
    CloseHandle(h.hProcess);
    CloseHandle(h.hThread);
}

#endif

//---------------------------
// RUN LOAD TEST
//---------------------------

using sc = std::chrono::steady_clock;

struct Result {
    enum Output { Success, Timeout, Incorrect, FailedToConnect };

    decltype(sc::now() - sc::now()) duration;
    Output                          output;
};

static std::unique_ptr<Client> create_client(Config const& cfg)
{
    if (cfg.client_type == ClientType::TCP)
        return std::make_unique<TCPClient>("127.0.0.1", 23456);
    else
        return std::make_unique<SSLClient>("127.0.0.1", 23457);
}


static std::string random_string(size_t n)
{
    static const char* c="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string r; r.reserve(n);
    while(n--) r+=c[rand()%62];
    return r;
}

int main(int argc, char* argv[])
{
    srand(time(nullptr));

    Config config = parse_args(argc, argv);
    std::vector<Result> results;
    std::mutex mutex;

    pid_t server_pid;
    if (config.run_server)
        server_pid = run_server(config.client_type);

    std::vector<std::jthread> t;
    for (size_t i = 0; i < config.n_threads; ++i) {
        t.emplace_back([&]{
            for (size_t j = 0; j < config.n_attempts; ++j) {
                std::string str = random_string(10) + "\n";
                std::string response;
                Result result;
                auto start = sc::now();
                std::unique_ptr<Client> client;

                try {
                    client = create_client(config);
                } catch (std::exception& e) {
                    result.output = Result::FailedToConnect;
                    goto skip;
                }
                client->send(str);
                response = client->recv_spinlock(11, 30000ms).value_or("");
                result = {
                    .duration = sc::now() - start,
                    .output = (response == str) ? Result::Success : (response.empty() ? Result::Timeout : Result::Incorrect),
                };
skip:
                {
                    std::lock_guard lock(mutex);
                    results.emplace_back(result);
                }
            }
        });
    }
    for (size_t i = 0; i < config.n_threads; ++i)
        t[i].join();

    // report
    using duration = decltype(results[0].duration);
    size_t success = 0, timeout = 0, incorrect = 0, failed_to_connect = 0;
    duration total_time = 0ms;
    duration best_time = 60000ms;
    duration worst_time = 0ms;
    for (auto const& result: results) {
        if (result.output == Result::Success) {
            ++success;
            total_time += result.duration;
            if (result.duration > worst_time)
                worst_time = result.duration;
            if (result.duration < best_time)
                best_time = result.duration;
        }
        if (result.output == Result::Timeout)
            ++timeout;
        if (result.output == Result::Incorrect)
            ++incorrect;
        if (result.output == Result::FailedToConnect)
            ++failed_to_connect;
    }

    printf("Out of %zu requests: %zu succeeded, %zu timed out, %zu returned an incorrect result, and %zu failed to connect.\n",
           results.size(), success, timeout, incorrect, failed_to_connect);
    if (success > 0) {
        printf("For the succeeded tests, average time was %0.2f ms. Best time was %0.2f ms, and worst time was %0.2f ms.\n",
               std::chrono::duration_cast<std::chrono::microseconds>(total_time / success).count() / 1000.0,
               std::chrono::duration_cast<std::chrono::microseconds>(best_time).count() / 1000.0,
               std::chrono::duration_cast<std::chrono::microseconds>(worst_time).count() / 1000.0
        );
    }

    kill_server(server_pid);

    return (timeout + incorrect + failed_to_connect) > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}