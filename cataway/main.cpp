
#include <algorithm>

#include <pistache/net.h>
#include <pistache/http.h>
#include <pistache/peer.h>
#include <pistache/http_headers.h>
#include <pistache/cookie.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <pistache/common.h>

#include <signal.h>

using namespace std;
using namespace Pistache;

void printCookies(const Http::Request& req) {
    auto cookies = req.cookies();
    std::cout << "Cookies: [" << std::endl;
    const std::string indent(4, ' ');
    for (const auto& c: cookies) {
        std::cout << indent << c.name << " = " << c.value << std::endl;
    }
    std::cout << "]" << std::endl;
}

namespace Generic {

    void handleReady(const Rest::Request&, Http::ResponseWriter response) {
        response.send(Http::Code::Ok, "1\n");
    }

}

class CatAwayEndpoint {
public:
    explicit CatAwayEndpoint(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    { }

    void init(size_t thr = 2) {
        auto opts = Http::Endpoint::options()
            .threads(static_cast<int>(thr));
        httpEndpoint->init(opts);
        // Server routes are loaded up
        setupRoutes();
    }

    // Server is started threaded.  
    void start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serveThreaded();
    }

    // When signaled server shuts down
    void stop(){
        httpEndpoint->shutdown();
    }

private:
    void setupRoutes() {
        using namespace Rest;
        Routes::Get(router, "/ready", Routes::bind(&Generic::handleReady));
        Routes::Get(router, "/auth", Routes::bind(&CatAwayEndpoint::doAuth, this));
        Routes::Post(router, "/settings/add/:addSetting/:value", Routes::bind(&CatAwayEndpoint::addSetting, this));
        // Routes::Post(router, "/settings/add/age/:value", Routes::bind(&CatAwayEndpoint::addAge, this));
        Routes::Get(router, "/settings/:resultSetting", Routes::bind(&CatAwayEndpoint::getSetting, this));
        Routes::Get(router, "/test", Routes::bind(&CatAwayEndpoint::test, this));
    }

    
    void doAuth(const Rest::Request& request, Http::ResponseWriter response) {
        // Function that prints cookies
        printCookies(request);
        // In the response object, it adds a cookie regarding the communications language.
        response.cookies()
            .add(Http::Cookie("lang", "en-US"));
        // Send the response
        response.send(Http::Code::Ok, "The CatAway setup has completed!");
    }

    // Endpoint to configure one of the CatAway's settings.
    void addSetting(const Rest::Request& request, Http::ResponseWriter response){
        // You don't know what the parameter content that you receive is, but you should
        // try to cast it to some data structure. Here, I cast the settingName to string.
        auto settingName = request.param(":addSetting").as<std::string>();

        // This is a guard that prevents editing the same value by two concurent threads. 
        Guard guard(CatAwayLock);
        
        
        string val = "";
        if (request.hasParam(":value")) {
            auto value = request.param(":value");
            val = value.as<string>();
        }
        
        // Setting the CatAway's setting to value
        int setResponse = cat.set(settingName, string(val));

        // Sending some confirmation or error response.
        if (setResponse == 1) {
            response.send(Http::Code::Ok, settingName + " was set to " + val);
        }
        else {
            response.send(Http::Code::Not_Found, settingName + " was not found and or '" + val + "' was not a valid value ");
        }

    }
    void addAge(const Rest::Request& request, Http::ResponseWriter response){
        Guard guard(CatAwayLock);
        
        
        string val = "";
        if (request.hasParam(":value")) {
            auto value = request.param(":value");
            val = value.as<string>();
        }
        
        // Setting the CatAway's setting to value
        int setResponse = cat.set("age", string(val));

        // Sending some confirmation or error response.
        if (setResponse == 1) {
            response.send(Http::Code::Ok,  "Age was set to " + val);
        }
        else {
            response.send(Http::Code::Not_Found, "Age was not found and or '" + val + "' was not a valid value ");
        }
    }

    // Setting to get the settings value of one of the configurations of the CatAway
    void getSetting(const Rest::Request& request, Http::ResponseWriter response){
        auto settingName = request.param(":resultSetting").as<std::string>();

        Guard guard(CatAwayLock);

        string valueSetting = cat.get(settingName);

        if (valueSetting != "") {

            // In this response I also add a couple of headers, describing the server that sent this response, and the way the content is formatted.
            using namespace Http;
            response.headers()
                        .add<Header::Server>("pistache/0.1")
                        .add<Header::ContentType>(MIME(Text, Plain));

            response.send(Http::Code::Ok, settingName + " is " + valueSetting);
        }
        else {
            response.send(Http::Code::Not_Found, settingName + " was not found");
        }
    }

    void test(const Rest::Request& request, Http::ResponseWriter response){
        response.send(Http::Code::Ok, "test");
    }

    // Defining the class of the CatAway. It should model the entire configuration of the CatAway
    class CatAway {
    public:
        explicit CatAway(){ }

        // Setting the value for one of the settings. Hardcoded for the defrosting option
        int set(std::string name, std::string value) {
            if(name == "weight") {
                weight = stof(value);
                return 1;
            } else if (name == "age") {
                age = stoi(value);
                return 1;
            } else if (name == "eatingSpeed") {
                eatingSpeed = value;
                return 1;
            } else if (name == "mSchedule"){
                schedule = value;
                return 1;
            }
            return 0;
        }

        // Getter
        string get(std::string name){
            if(name == "weight") {
                return std::to_string(weight);
            } else if (name == "age") {
                return std::to_string(age);
            } else if (name == "eatingSpeed") {
                return eatingSpeed;
            } else if (name == "mSchedule"){
                return schedule;
            }
            return 0;
        }

    private:
       float weight; 
       int age;
       string eatingSpeed;
       string schedule;
    };

    // Create the lock which prevents concurrent editing of the same variable
    using Lock = std::mutex;
    using Guard = std::lock_guard<Lock>;
    Lock CatAwayLock;

    // Instance of the CatAway model
    CatAway cat;

    // Defining the httpEndpoint and a router.
    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

int main(int argc, char *argv[]) {

    // This code is needed for gracefull shutdown of the server when no longer needed.
    sigset_t signals;
    if (sigemptyset(&signals) != 0
            || sigaddset(&signals, SIGTERM) != 0
            || sigaddset(&signals, SIGINT) != 0
            || sigaddset(&signals, SIGHUP) != 0
            || pthread_sigmask(SIG_BLOCK, &signals, nullptr) != 0) {
        perror("install signal handler failed");
        return 1;
    }

    // Set a port on which your server to communicate
    Port port(8080);

    // Number of threads used by the server
    int thr = 2;

    if (argc >= 2) {
        port = static_cast<uint16_t>(std::stol(argv[1]));

        if (argc == 3)
            thr = std::stoi(argv[2]);
    }

    Address addr(Ipv4::any(), port);

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;

    // Instance of the class that defines what the server can do.
    CatAwayEndpoint stats(addr);

    // Initialize and start the server
    stats.init(thr);
    stats.start();


    // Code that waits for the shutdown sinal for the server
    int signal = 0;
    int status = sigwait(&signals, &signal);
    if (status == 0)
    {
        std::cout << "received signal " << signal << std::endl;
    }
    else
    {
        std::cerr << "sigwait returns " << status << std::endl;
    }

    stats.stop();
}