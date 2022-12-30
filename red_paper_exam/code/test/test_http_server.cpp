#include "rest_rpc.hpp"
#include "rest_rpc/rpc_client.hpp"
#include <cofish/http/http_server.h>
#include "double_mean.h"
#include <cofish/util/ex_util.hpp>
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
    long num; /* 红包剩余人数，同时作为CAS比较的对象 */
    MSGPACK_DEFINE(money, num);
} red_paper_t;

struct dummy {
    red_paper_t rp { 100.0, 10 };
    std::unordered_map<unsigned long, red_paper_t> cache;
    long count = 10;
    bool lock = false;
    int double_x(rpc_conn conn, int x) { return 2 * x; }
    long get(rpc_conn conn) { return count; }
    void sub(rpc_conn conn, long s) { 
        count = s;
        printf("count: %ld\n", count);
    }
    red_paper_t query(rpc_conn conn) { 
        return { rp.money, rp.num };
    }
    red_paper_t select(rpc_conn conn, unsigned long id) {
        if(cache.find(id) == cache.end()) return { -1, 0 };
        auto p = &cache[id];
        return { p->money, p->num };
    }
    bool grab(rpc_conn conn, double cal)
    {
        if(rp.num <= 0 || rp.money < cal) return false;
        rp.money -= cal;
        rp.num--;
        return true;
    }
    bool grab_cas(rpc_conn conn, unsigned long id, double cal, int cas_num)
    {
        if(cache.find(id) ==  cache.end()) return false;
        red_paper_t *p = &cache[id];
        if(p->num != cas_num) return false;
        p->money -= cal;
        p->num--;
        return true;
    }
    unsigned long insert(rpc_conn conn, double money, int num) {
        unsigned long now = TimeStampUs();
        if(cache.find(now) != cache.end()) return 0;
        cache[now] = { money, num };
        printf("------------------- cache begin --------------\n");
        for(auto it = cache.begin(); it != cache.end(); it++)
            printf("%ld ---> %.2lf, %ld\n", it->first, it->second.money, it->second.num);
        printf("------------------- cache end --------------\n");
        return now;
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
        server.register_handler("grab_cas", &dummy::grab_cas, &d);
        server.register_handler("insert", &dummy::insert, &d);
        server.register_handler("select", &dummy::select, &d);
        server.run();
        return 0;
    }

    MultiProcess(process_cnt, [](){
        IoManager iom;
        char str[200];
        rpc_client cli;
        red_paper_t rp_cache{ -1, 0 };
        std::unordered_map<unsigned long, red_paper_t> cache;

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
        servlet_dispatch->SetServlet("/insert", [&](HttpRequest::Ptr req, HttpResponse::Ptr rsp, HttpSession::Ptr session) {
            unsigned long id = 0;
            if(!(id = cli.call<unsigned long>("insert", 100.0, 10))) {
                rsp->SetBody("insert failed!!!!");
            } else {
                snprintf(str, 200, "%ld\n", id); 
                rsp->SetBody(str);
            }
            return 0;
        });
        servlet_dispatch->SetServlet("/grab", [&](HttpRequest::Ptr req, HttpResponse::Ptr rsp, HttpSession::Ptr session){
            if(rp_cache.money == -1) {
                auto start_time = std::chrono::duration_cast<std::chrono::microseconds>(
                                                     std::chrono::system_clock::now().time_since_epoch()).count();
                red_paper_t rp = cli.call<red_paper_t>("query");
                rp_cache.money = rp.money;
                rp_cache.num = rp.num;
                auto end_time = std::chrono::duration_cast<std::chrono::microseconds>(
                                                     std::chrono::system_clock::now().time_since_epoch()).count();
                printf("query spend time: %ldus\n", end_time - start_time);
            }
            snprintf(str, 200, "red paper is over!\n");
            if(rp_cache.num > 0) {
                while(!cli.call<bool>("lock"));
                red_paper_t rp = cli.call<red_paper_t>("query");
                rp_cache.money = rp.money;
                rp_cache.num = rp.num;
                if(rp.num > 0) {
                    // printf("query: %.2lf, %ld\n", rp.money, rp.num);
                    double cal = std::calculate_one_paper_by_double_mean(rp.money, rp.num);
                    if(cli.call<bool>("grab", cal))
                        snprintf(str, 200, "grab money: %.2lf\n", cal);
                    rp_cache.money = rp.money - cal;
                    rp_cache.num = rp.num - 1;
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
            unsigned long id = std::stoul(path);
            snprintf(str, 200, "red paper is over!\n");
            red_paper_t *p = nullptr;
            std::unordered_map<unsigned long, red_paper_t>::iterator it;
            if((it = cache.find(id)) != cache.end()) {
                p = &cache[id];
            }
            // 如果缓存不存在或者还有剩余名额，则开始CAS抢红包
            if(p == nullptr || p->num > 0) {
                bool cas_result = false;
                unsigned int count = 5;
                while(!cas_result && --count) {
                    // 有限次数重试CAS操作
                    red_paper_t rp = cli.call<red_paper_t>("select", id);
                    if(rp.money == -1) {
                        if((it = cache.find(id)) != cache.end()) cache.erase(it);
                        p = nullptr;
                        rsp->SetBody("red paper not found!!!\n");
                        return 0;
                    }
                    // 更新缓存
                    if((it = cache.find(id)) != cache.end()) {
                        p = &cache[id];
                        p->money = rp.money;
                        p->num = rp.num;      // 这个num字段也是CAS比较字段
                    }
                    else cache[id] = { rp.money, rp.num };
                    p = &cache[id];
                    if(rp.num > 0) {
                        // 拆包
                        double cal = std::calculate_one_paper_by_double_mean(rp.money, rp.num);
                        if((cas_result = cli.call<bool>("grab_cas", id, cal, rp.num))) {
                            snprintf(str, 200, "grab money: %.2lf\n", cal);
                            std::cout << str;
                        }
                        // 更新缓存
                        p->money = rp.money - cal;
                        p->num = rp.num - 1;
                    }
                }
            }
            rsp->SetBody(str);
            // rsp->SetBody("Servlet path is \"/abc\"\n");
            return 0;
        });
        servlet_dispatch->SetGlobServlet("/add", [&](HttpRequest::Ptr req, HttpResponse::Ptr rsp, HttpSession::Ptr session){
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
