#include "rest_rpc.hpp"
#include "rest_rpc/rpc_client.hpp"
#include <cofish/http/http_server.h>
#include "double_mean.h"
#include <cstdio>
#include <memory>
#include <string>

#include <map>
#include <unordered_map>

using namespace fish;
using namespace fish::http;
using namespace rest_rpc;
using namespace rest_rpc::rpc_service;

int process_cnt = 0;
int client_handle_co_cnt = 0;
fish::Socket::Ptr g_listen_sock = nullptr;

void OnMainInt(int) {
    FISH_LOGDEBUG("OnMainInt");
    if (g_listen_sock) {
        g_listen_sock->Close();
    }
    kill(0, SIGINT);
    wait(nullptr);
    exit(-1);
}

typedef struct red_paper_t {
    double money;
    long num;
    MSGPACK_DEFINE(money, num);
} red_paper_t;

struct dummy {
    red_paper_t rp { 100.0, 10 };
    long count = 10;
    bool lock = false;
    std::unordered_map<std::string, red_paper_t> cache;
    int double_x(rpc_conn conn, int x) { return 2 * x; }
    long get(rpc_conn conn) { return count; }
    void sub(rpc_conn conn, long s) { 
        count = s;
        printf("count: %ld\n", count);
    }
    red_paper_t query(rpc_conn conn) { 
        return { rp.money, rp.num };
    }
    bool grab(rpc_conn conn, double cal)
    {
        if(rp.num <= 0 || rp.money < cal) return false;
        rp.money -= cal;
        rp.num--;
        return true;
    }
    bool onLock(rpc_conn conn) { if(lock) return false; else { lock = true; return true;} }
    void unLock(rpc_conn conn) { lock = false; }
};

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: http_server process_count client_handle_co_cnt" << std::endl;
        exit(-1);
    }

    process_cnt = atoi(argv[1]);
    client_handle_co_cnt= atoi(argv[2]);

    fish::SetLogLevel(5);

    g_listen_sock = Socket::CreateTCP();
    assert(g_listen_sock);
    assert(g_listen_sock->Init());
    assert(g_listen_sock->Bind(IPv4Address::create("0.0.0.0", 9000)));
    assert(g_listen_sock->Listen(128));

    if(!fork())
    {
        // rpc_server server = { 9001, std::thread::hardware_concurrency() };
        rpc_server server = { 9001, 1 };
        dummy d;
        server.register_handler("add", &dummy::double_x, &d);
        server.register_handler("get", &dummy::get, &d);
        server.register_handler("sub", &dummy::sub, &d);
        server.register_handler("lock", &dummy::onLock, &d);
        server.register_handler("unlock", &dummy::unLock, &d);
        server.register_handler("query", &dummy::query, &d);
        server.register_handler("grab", &dummy::grab, &d);
        server.run();
        return 0;
    }

    MultiProcess(process_cnt, [](){
        IoManager iom;
        char str[200];
        rpc_client cli;
        cli.enable_auto_heartbeat();
        cli.enable_auto_reconnect();
        cli.connect("127.0.0.1", 9001);
        auto http_server = HttpServer::Create("none", true);
        if (!http_server) {
            FISH_LOGFATAL("HttpServer::Create fail");
            exit(-1);
        }
        if (!http_server->Init(g_listen_sock, &iom, client_handle_co_cnt)) {
            FISH_LOGFATAL("HttpServer::Init fail");
            exit(-1);
        }
        if (!http_server->Start()) {
            FISH_LOGFATAL("HttpServer::Start fail");
            exit(-1);
        }

        auto servlet_dispatch = http_server->GetServletDispatch();
        servlet_dispatch->SetServlet("/", [&](HttpRequest::Ptr req, HttpResponse::Ptr rsp, HttpSession::Ptr session){
            long count = cli.call<long>("get");
            if(count) {
                while(!cli.call<bool>("lock"));
                count = cli.call<long>("get");
                if(count) {
                    cli.call<void>("sub", count - 1);
                }
                cli.call<void>("unlock");
            }
            snprintf(str, 200, "get count: %ld\n", count);
            rsp->SetBody(str);
            return 0;
        });
        servlet_dispatch->SetServlet("/grab", [&](HttpRequest::Ptr req, HttpResponse::Ptr rsp, HttpSession::Ptr session){
            red_paper_t rp = cli.call<red_paper_t>("query");
            snprintf(str, 200, "red paper is over!\n");
            if(rp.num > 0) {
                while(!cli.call<bool>("lock"));
                rp = cli.call<red_paper_t>("query");
                if(rp.num > 0) {
                    // printf("query: %.2lf, %ld\n", rp.money, rp.num);
                    double cal = std::calculate_one_paper_by_double_mean(rp.money, rp.num);
                    if(cli.call<bool>("grab", cal))
                        snprintf(str, 200, "grab money: %.2lf\n", cal);
                }
                std::cout << str;
                cli.call<void>("unlock");
            }
            rsp->SetBody(str);
            // rsp->SetBody("Servlet path is \"/abc\"\n");
            return 0;
        });
        servlet_dispatch->SetGlobServlet("*", [&](HttpRequest::Ptr req, HttpResponse::Ptr rsp, HttpSession::Ptr session){
            std::string path = req->GetPath();
            path.erase(0, 1);
            int num = std::stoi(path);
            cli.connect();
            int result = cli.call<int>("add", num);
            snprintf(str, 200, "result: %d\n", result);
            rsp->SetBody(str);
            return 0;
        });

        iom.Start();
        }, OnMainInt);
    return 0;
}
