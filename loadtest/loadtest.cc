#include "neblina.hh"

#include <getopt.h>
#include <cstdio>
#include <cstdlib>

#include <chrono>
#include <string>
#include <thread>
#include <vector>

//------------------------------------------------------
// PARSE OPTIONS
//------------------------------------------------------

enum class ClientType { None, TCP, SSL };

struct Config {
    ClientType client_type = ClientType::None;
    size_t n_attempts = 1;
    size_t n_threads  = 1;
};

static void show_help()
{
    fprintf(stderr, "Usage: neblina-load-test [--tcp|--ssl] [--attempts N] [--threads N]\n");
    fprintf(stderr, "  -t, --tcp       Use TCP client\n");
    fprintf(stderr, "  -s, --ssl       Use SSL client\n");
    fprintf(stderr, "  -a, --attempts  Number of attempts (default 1)\n");
    fprintf(stderr, "  -n, --threads   Number of threads  (default 1)\n");
    fprintf(stderr, "Server must be running.\n");
    exit(EXIT_FAILURE);
}

Config parse_args(int argc, char** argv)
{
    Config cfg {};

    const option long_opts[] = {
            {"tcp",      no_argument,       nullptr, 't'},
            {"ssl",      no_argument,       nullptr, 's'},
            {"attempts", required_argument, nullptr, 'a'},
            {"threads",  required_argument, nullptr, 'n'},
            {"help",     no_argument,       nullptr, 'h'},
            {nullptr,    0,                 nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "ts:a:n:h", long_opts, nullptr)) != -1) {
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
// RUN LOAD TEST
//---------------------------

using sc = std::chrono::steady_clock;

struct Result {
    enum Output { Success, Timeout, Incorrect };

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

    std::jthread t[config.n_threads];
    for (size_t i = 0; i < config.n_threads; ++i) {
        t[i] = std::jthread([&]{
            for (size_t j = 0; j < config.n_attempts; ++j) {
                std::string str = random_string(10) + "\n";
                auto start = sc::now();
                std::unique_ptr<Client> client = create_client(config);
                client->send(str);
                std::string response = client->recv_spinlock(11, 30000ms).value_or("");
                Result result = {
                    .duration = sc::now() - start,
                    .output = (response == str) ? Result::Success : (response.empty() ? Result::Timeout : Result::Incorrect),
                };
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
    size_t success = 0, timeout = 0, incorrect = 0;
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
    }

    printf("Out of %zu request: %zu succeeded, %zu timed out, %zu returned an incorrect result.\n", results.size(),
           success, timeout, incorrect);
    if (success > 0) {
        printf("For the succeeded, average time was %zu ms. Best time was %zu ms, and worst time was %zu ms.\n",
               (total_time / success).count(),
        );
    }
}