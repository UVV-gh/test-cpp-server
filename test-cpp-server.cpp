#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace std::string_literals;

#include <iostream>
#include <string>
#include <condition_variable>
#include <mutex>
#include <signal.h>

static std::condition_variable gCondition;
static std::mutex gMutex;

class InterruptHandler {
public:
    static void hookSIGINT() {
        signal(SIGINT, handleUserInterrupt);        
    }

    static void handleUserInterrupt(int signal){
        if (signal == SIGINT) {
            std::cout << "SIGINT trapped ..." << '\n';
            gCondition.notify_one();
        }
    }

    static void waitForUserInterrupt() {
        std::unique_lock<std::mutex> lock { gMutex };
        gCondition.wait(lock);
        std::cout << "user has signaled to interrup program..." << '\n';
        lock.unlock();
    }
};


std::string getFileContent(const std::string& fileName)
{
    std::ifstream in(fileName, std::ios::in | std::ios::binary);
    if (!in) {
        throw std::runtime_error("File error");
    }

    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();

    return contents;
}

void handleGet(http_request request)
{
    auto urlPaths = uri::split_path(request.request_uri().path());
    auto urlParams = uri::split_query(request.request_uri().query());

#if 0
    std::cout << "Paths\n";
    for (auto& s : urlPaths) {
        std::cout << s << ' ';
    }
    std::cout << '\n';

#endif

    if (urlPaths.size() < 5) {
        request.reply(status_codes::BadRequest);
        return;
    }

#if 0
    std::cout << "Params\n";
    for (const auto& [key, val] : urlParams) {
        std::cout << key << " = " << val << '\n';
    }

#endif

    std::string requestType = urlPaths[3];
    std::string main = urlPaths[4];

    auto mainFileContent = getFileContent("./data/Format_" + main + ".json");
    auto formatsJson = json::value::parse(mainFileContent);

#if 0
    if (requestType == "valuebyobject") {
        handleGetValueRequest(request, formatsJson);
    }
    else if (requestType == "formatbyobject") {
        handleGetFormatRequest(request, formatsJson);
    }
#endif

    request.reply(status_codes::OK, formatsJson);
}

void handleGetFormatRequest(http_request request)
{
    auto answer = json::value::object();
    request.reply(status_codes::OK, answer);
}

int main(int argc, char** argv)
{
    const std::string baseUri("http://0.0.0.0:8079/api/v1/parameter/");

    http_listener listener(baseUri);
    listener.support(methods::GET, handleGet);

    InterruptHandler::hookSIGINT();

    try {
        listener.open().wait();
        InterruptHandler::waitForUserInterrupt();

    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
