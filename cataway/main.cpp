
#include <algorithm>

#include <pistache/net.h>
#include <pistache/http.h>
#include <pistache/peer.h>
#include <pistache/http_headers.h>
#include <pistache/cookie.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <pistache/common.h>

#include <mosquitto.h>

#include <ctime>
#include <signal.h>

using namespace std;
using namespace Pistache;

float globalWeight;


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
        Routes::Get(router, "/settings/:resultSetting", Routes::bind(&CatAwayEndpoint::getSetting, this));
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

    // Defining the class of the CatAway. It should model the entire configuration of the CatAway
    class CatAway {
    public:
        explicit CatAway(){ }
        void setRecFood()
        {
            float cups = 0;
            if(this->age > 1.0)
            {
                if(this->weight <= 1.8)
                cups = 0.25;
                else if(this->weight <= 3.6)
                cups = 0.5;

                else if(this->weight <= 5.4)
                cups = 0.75;

                else if(this->weight <= 7.2)
                cups = 1.0;

                else if(this->weight <= 9.0)
                cups = 1.25;
            }
            else if(this->age >= 0.17 && this->age < 0.42)  //[2, 5) luni
            {
                    if(this->weight <= 0.9)
                cups = 0.5;
                else if(this->weight <= 1.8)
                cups = 0.75;

                else if(this->weight <= 2.7)
                cups = 1.0;

                else if(this->weight <= 3.6)
                cups = 1.25;

                else if(this->weight <= 4.5)
                cups = 1.5;

                else if(this->weight <= 5.4)
                cups = 1.75;

                else if(this->weight <= 6.3)
                cups = 2.0;

                else if(this->weight <= 8.1)
                cups = 2.25;

                else if(this->weight <= 9.0)
                cups = 2.5;
            }
            else if(this->age >= 0.42 && this->age < 0.58)  //[5, 7) luni
            {
                if(this->weight <= 0.9)
                cups = 0.25;

                else if(this->weight <= 2.7)
                cups = 0.5;

                else if(this->weight <= 4.5)
                cups = 0.75;

                else if(this->weight <= 6.3)
                cups = 1.0;

                else if(this->weight <= 9.0)
                cups = 1.25;
            }
            else if(this->age >= 0.58 && this->age <= 1.0)  //[7, 12] luni
            {
                if(this->weight <= 1.8)
                    cups = 0.25;

                else if(this->weight <= 4.5)
                    cups = 0.5;

                else if(this->weight <= 7.2)
                    cups = 0.75;

                else if(this->weight <= 9.0)
                    cups = 1.0;
            }
        this->recFoodG = 224*cups;   //224 g per cup
        }

        void setBreaks()
        {
            if(this->eatingSpeed == "slow")
                this->nrBreaks = 0;
            else if(this->eatingSpeed == "medium")
                this->nrBreaks = 1;
            else if(this->eatingSpeed == "fast")
                this->nrBreaks = 2;
        }

        void setNextFoodRefill()
        {
            if(!recFoodG)
            {
                setRecFood();
            }
            if(feedingSchedule == "")
            {
                feedingSchedule = "08:00 19:00 ";
            }
            //in g
            int cantitate = recFoodG * feedingSchedule.length()/6;  //pe zi

            

        }

        // Setting the value for one of the settings. Hardcoded for the defrosting option
        int set(std::string name, std::string value) {
            struct tm tm_;
             if(name == "weight") {
                weight = stof(value);
                globalWeight = weight;
                return 1;
            } else if (name == "age") {
                age = stoi(value);
                return 1;
            } else if (name == "eatingSpeed") {
                eatingSpeed = value;
                return 1;
            } else if (name == "feedingSchedule"){
                feedingSchedule = value;
                return 1;
            } else if (name == "waterBowlWeightG"){
                waterBowlWeightG = stoi(value);
                return 1;
            } else if (name == "waterRefSchedule"){
                waterRefSchedule = value;
                return 1;
            } else if (name == "foodExpDate"){
                strptime(value.c_str(), "%d.%m.%Y %H:%M", &tm_);
                foodExpDate = mktime(&tm_);
                return 1;
            } else if (name == "emptyFoodTank"){
                emptyFoodTank = (value == "1");
                return 1;
            } else if (name == "emptyWaterTank"){
                emptyWaterTank = (value == "1");
                return 1;
            } else if (name == "lastConsumedWater"){
                lastConsumedWater = stoi(value);
                if(lastConsumedWater <= this->currentQuantityWaterMl)
                    this->currentQuantityWaterMl -= lastConsumedWater;
                else
                {
                    this->currentQuantityWaterMl = 0;
                    this->emptyWaterTank = true;
                }
                return 1;
            } else if (name == "lastConsumedFood"){
                lastConsumedFood = stoi(value);
                if(!this->Expired()) {
                    if(lastConsumedWater <= this->currentQuantityFoodG){
                        this->currentQuantityFoodG -= lastConsumedWater;
                    } else {
                    this->currentQuantityFoodG = 0;
                    this->emptyFoodTank = true;
                    this->refillFood = true;
                  } 
                } else {
                    this->refillFood = true;
                    this->expiredFood = true;
                }
                return 1;
            } 
            return 0;
        }

        bool Expired()
        {
            if(!foodExpDate)
                return false;
            time_t now = time(0);
            tm local_tm = *localtime(&now);
            tm exp_date = *localtime(&foodExpDate);

            if(local_tm.tm_year > exp_date.tm_year){
                return true;
            }else if(local_tm.tm_year == exp_date.tm_year){
                if(local_tm.tm_mon > exp_date.tm_mon){
                    return true;
                }else if(local_tm.tm_mon == exp_date.tm_mon){
                    if(local_tm.tm_mday < exp_date.tm_mday){
                        return true;
                    }
                }
            }
            return false;
        }
        // Getter
        string get(std::string name){
            struct tm *tm_;
            if(name == "weight") {
                return to_string(weight);
            } else if (name == "age") {
                return to_string(age);
            } else if (name == "eatingSpeed") {
                return eatingSpeed;
            } else if (name == "feedingSchedule"){
                return feedingSchedule;
            } else if (name == "waterBowlWeightG"){
                return to_string(waterBowlWeightG);
            } else if (name == "waterRefSchedule"){
                return waterRefSchedule;
            } else if (name == "foodExpDate"){
                tm *tm_ = gmtime(&foodExpDate);
                string someString(asctime(tm_));
                return someString;
            } else if (name == "emptyFoodTank"){
                return emptyFoodTank ? "true" : "false";
            } else if (name == "emptyWaterTank"){
                return emptyWaterTank ? "true" : "false";
            } else if (name == "recFoodG"){
                return to_string(recFoodG);
            } else if (name == "nrBreaks"){
                return to_string(nrBreaks);
            }   else if (name == "currentQuantityWaterMl"){
                return to_string(currentQuantityWaterMl);
            }   else if (name == "refreshWater"){
                return refreshWater ? "true" : "false";
            }   else if (name == "currentQuantityFoodG"){
                return to_string(currentQuantityFoodG);
            }   else if (name == "refillFood"){
                return refillFood ? "true" : "false";
            }   else if (name == "nextFoodRefill"){
                tm *tm_ = gmtime(&nextFoodRefill);
                string someString(asctime(tm_));
                return someString;
            }   else if (name == "nextWaterRefill"){
                tm *tm_ = gmtime(&nextWaterRefill);
                string someString(asctime(tm_));
                return someString;
            }   else if (name == "lastConsumedWater"){
                return to_string(lastConsumedWater);
            }   else if (name == "lastConsumedFood"){
                return to_string(lastConsumedFood);
            }     
            return 0;
        }

    private:
       float weight; 
       float age;
       string eatingSpeed;
       string feedingSchedule = "";
       int waterBowlWeightG;                                //water bowl weight in g
       string waterRefSchedule;                             //water refreshment schedule
       time_t foodExpDate;
       bool emptyFoodTank;
       bool emptyWaterTank;
       bool expiredFood = false;
       int recFoodG = -1;                                      //recommended quantity of food in g
       int nrBreaks;                                        //number of breaks
       int currentQuantityWaterMl;                       //quantity of water in tank in ml
       bool refreshWater;                                   //true if needs to be refreshed
       int currentQuantityFoodG;                          //food in tank
       bool refillFood;                                     //true if needs to be refilled
       time_t nextFoodRefill;
       time_t nextWaterRefill;
       const int tankSizeFoodG = 1000;                       //in g
       const int tankSizeWaterMl = 2000;                      //in ml
       int lastConsumedWater;                                 //in ml
       int lastConsumedFood;                                 //in g
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


void pistacheThread(int argc, char** argv) {
    // This code is needed for gracefull shutdown of the server when no longer needed.
    sigset_t signals;
    if (sigemptyset(&signals) != 0
            || sigaddset(&signals, SIGTERM) != 0
            || sigaddset(&signals, SIGINT) != 0
            || sigaddset(&signals, SIGHUP) != 0
            || pthread_sigmask(SIG_BLOCK, &signals, nullptr) != 0) {
        perror("install signal handler failed");
        return;
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

void printWeight(struct mosquitto *mosq)
{
	int rc;
    
    sleep(1);
    string msg = "The weight of the cat is:" + to_string(globalWeight);
   int n = msg.length();
   char msg_array[50];
   strcpy(msg_array, msg.c_str());

	rc = mosquitto_publish(mosq, NULL, "setting/weight", n+1, msg_array, 0, false);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
	}
}

void mosquittoThread(int arg, char** argv) {
    int rc;
   struct mosquitto *mosq;

   mosquitto_lib_init();

   mosq = mosquitto_new("publisher-test", true, NULL);

   rc = mosquitto_connect(mosq, "localhost", 1883, 60);

   if (rc != 0) {
       printf("Client could not connect!");
       mosquitto_destroy(mosq);
       return;
   }

   rc = mosquitto_loop_start(mosq);
   float currentWeight;
   while(1) {
       if(currentWeight != globalWeight) {
           printWeight(mosq);
           currentWeight = globalWeight;
       }
   }
   

   mosquitto_lib_cleanup();
}


int main(int argc, char *argv[]) {
    thread pistacheThr(pistacheThread, argc, argv);
    thread mosquittoThr(mosquittoThread, argc, argv);

    pistacheThr.join();
    mosquittoThr.join();
    return 0;
}
