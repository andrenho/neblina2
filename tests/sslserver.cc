#include "doctest.h"

#include <atomic>
#include <memory>
#include <thread>

#include "neblina.hh"

#define PORT 23457

class EchoProtocol : public Protocol {
public:
    class Session : public ::Session {
    public:
        explicit Session(std::unique_ptr<Connection> connection) : ::Session(std::move(connection)) {}

    protected:
        std::string process(std::string const &data) override {
            return data;
        }
    };

    [[nodiscard]] std::unique_ptr<::Session> create_session(std::unique_ptr<Connection> connection) const override {
        return std::make_unique<Session>(std::move(connection));
    }
};

static const char* private_key = R"(
-----BEGIN RSA PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC+d/a8Z5D/M036rvF0tPbukhdMtrRCIpuFT5G7iigasf2e+lQsQ3/AUfjXmYfJ8hTfo4YDqBkD3GxxlB7sCBpP5jDFHzWcW07pppqcEH1CpaeRVvcYSKs3qEu7LRQ98ngzc3fVOOD8Af5DL73ctTByz6pkdLieHFcsx0QtgTwiuJYmo+loS+6znKuPUmgo3zV+KfJktYZ+r0P4x+ruilcUm17vd1QhKNiaoGRhL4nlFj+hn3SxXUWdNxGMQFGp5bdHr8JbJgwqYoFl3Om4clmeM9injT2WaHADlo9Qm3WT398Y5fwbSGCtulTqbLU6VZBkHViJpIvQ3oN+WKdvwm5tAgMBAAECggEADyFMck7vz7XtndpDx/eQHgdw3mEtFIvXOtHFS9Z/tQ8qjHRqN1HtVgOs9sN1a+9VBSbiQGwcAJiD7MnNYwaJwIsSmLYP9imyrRDU+0y8Bo1OMFduLXmoUDWIPogoMXuPXNbxT/jCtoIaBpBGTr/950Dj0jMLb1PlAOObYNoBQuivkPyg521q1h/ft3f3jahtZruW8Yer0XQnR43+u1yvZ9SDt58VbO/JH3ZUeQbiXnr6dgJCadd+7Pws1bqBt9rOyaaYtm26vhgXJ0yHZuhify+qgYpZ+dIIu2KHt+632SfDbGdkar6QSEOmmRf/U4cIr3OxJ3k+wLnpddVbCZr4QQKBgQDHZzktyk6EqaSeJMQGRmoFDdGyCr1MumDn/+5rTBxd0/yYkCdeFSZqD2skbR9n674bT3kYhx22jWvkYnbupjkUVSTWNAQZ0Eg9juzWz1hx7dVMdTf3XPZWwofzlvXdfKEHdKeYcYEpeX1RkOLsSbP5gGCHX+KuGeeekMdsMlWsuQKBgQD0h4uvUiXSgdH/4GSIwCKfg94FSZ0WzEYP3XI4EZdwU5r2KaxbWCtwpWU/Mr4Ksl58+3hDYFgbrP+BpiGIqfR/Uqi0PaHwMm1n9p8ZVLRXPJsL/WEyvjcQCbkwHBJguiEDsXsHmkoRARnDF4R0FsXFYmQs9n+RfxHuSJj1RGg9VQKBgBpT4bPPC1DuySJsPkmJ0IO8UFyOmHHiBdt76jHWicRFsgs+tUPxzPGh+YaEtenVsXMPqjB33o902rL74TIfVlf96fAyS/xQiMk/7rtqAwZ/3XLL40jQyGGqK6MY4nuM8gH3AA3bpq8gMFCtrZh/HrkpnxUgruBH4VIJUZtShgdZAoGBAJMPaayY5PJC7P7L/upXUvtIHSBk8zqgw4oLDgQ2YDNHtKxSx7tvvUBT2B438B8Swsy7l8o9iP+r/UYFmalNPSPNnHQ5lMiWK0IZCfn7/TFx23VMf0Y7K3ZTwrHyUAMoLW7BMFKUMfMkDFBONnRytaJN2B1KadIg77G0q3jmGSK5AoGAGnS10s5TDbBSAIcB7AmiRCBjJJBL/SdBlOkOvQ+xsxw4u7G/3jPnRywPJSFFO75nrDv2hpF9brw2x/QAjTw3K7jd8iL7s2PgVaG+iZEikguiBSytDH0rjXv1PfHSj/bqS+yptqVcMd8wNGDlpX1h8+tsXVRuhn21ELzk1tJTHKI=
-----END RSA PRIVATE KEY-----
)";

static const char* certificate_key = R"(
-----BEGIN CERTIFICATE-----
MIIDDjCCAfagAwIBAgIGAZpGjphIMA0GCSqGSIb3DQEBCwUAMEgxFTATBgNVBAMMDGdhbWVzbWl0aC51azELMAkGA1UEBhMCVVMxIjAgBgkqhkiG9w0BCQEWE2FuZHJlLm5ob0BnbWFpbC5jb20wHhcNMjUxMTAyMjE1MjIxWhcNMjYxMTAyMjE1MjIxWjBIMRUwEwYDVQQDDAxnYW1lc21pdGgudWsxCzAJBgNVBAYTAlVTMSIwIAYJKoZIhvcNAQkBFhNhbmRyZS5uaG9AZ21haWwuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvnf2vGeQ/zNN+q7xdLT27pIXTLa0QiKbhU+Ru4ooGrH9nvpULEN/wFH415mHyfIU36OGA6gZA9xscZQe7AgaT+YwxR81nFtO6aaanBB9QqWnkVb3GEirN6hLuy0UPfJ4M3N31Tjg/AH+Qy+93LUwcs+qZHS4nhxXLMdELYE8IriWJqPpaEvus5yrj1JoKN81finyZLWGfq9D+Mfq7opXFJte73dUISjYmqBkYS+J5RY/oZ90sV1FnTcRjEBRqeW3R6/CWyYMKmKBZdzpuHJZnjPYp409lmhwA5aPUJt1k9/fGOX8G0hgrbpU6my1OlWQZB1YiaSL0N6Dflinb8JubQIDAQABMA0GCSqGSIb3DQEBCwUAA4IBAQB0chKAQ1TSCzHD4VHxJ8wZU/yq65pKwLyiarQJdZk4ItKB0ZBu+X2I7uJhj+NrsgCStFuIDmsyJcZr/4KI+FE9QOvqA64kQ6aJCsXX+EybAoCC3JO7pV7eCkfrMT2g3Ru4qIFw46XmHaC9x8bFWlTjdJaQIbtx0OFx8JR7knQLenGdiWG58hT2uQOyMZIPa/T/4Q+Wf8R/HQoARhKi+Z4ppclEU9khwPTChTae1Gz3rvctMHqkrguYbErFaWjfugsviF1OKTIK7VAYPOvLTWuEfS2O4Nrfb344xcE+GM5IimFmjfwdVYy38UKFdAIeEzl1krqmApaGswmvH4aFO9iy
-----END CERTIFICATE-----
)";


TEST_SUITE("SSL Server")
{
    TEST_CASE("SSL: Multi-threaded server and single-threaded client")
    {
        logging_verbose = true;
        logging_dest = stderr;

        std::jthread t([](std::stop_token const& st) {
            SSLServer server(PORT, false, std::make_unique<EchoProtocol>(), 2, certificate_key, private_key);
            std::this_thread::sleep_for(200ms);
            while (!st.stop_requested()) {
                server.iterate();
            }
        });

        std::this_thread::sleep_for(400ms);

        SSLClient client1("127.0.0.1", PORT);
        SSLClient client2("127.0.0.1", PORT);
        SSLClient client3("127.0.0.1", PORT);

        client1.send("hello\r\n");
        client2.send("hellw\r\n");
        client3.send("helly\r\n");
        std::this_thread::sleep_for(200ms);

        std::string response = client1.recv_spinlock(7, 1000ms).value_or("");
        CHECK(response == "hello\r\n");

        response = client2.recv_spinlock(7, 1000ms).value_or("");
        CHECK(response == "hellw\r\n");

        response = client3.recv_spinlock(7, 1000ms).value_or("");
        CHECK(response == "helly\r\n");
    }
}