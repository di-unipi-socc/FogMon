#include "uiconnection.hpp"

#include <iostream>
#include <string>
#include <curl/curl.h>

using namespace std;

UIConnection::UIConnection(Message::node myNode, string ip, int session) {
    this->ip = ip;
    this->myNode = myNode;
    this->session = session;
}

UIConnection::~UIConnection() {

}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool sendToInterface(string ip,string str) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        string link = "http://"+ip+"/data";
        curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, str.c_str());
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        std::cout << readBuffer << std::endl;
    }
}

bool UIConnection::sendTopology(vector<Report::report_result> report) {   
    if(this->ip != "") {
        Message m;
        m.setSender(myNode);
        m.setType((Message::Type)0);
        m.setCommand((Message::Command)0);
        m.setArgument((Message::Argument)this->session);

        Report r;
        r.setReports(report);

        m.setData(r);
        m.buildString();
        return sendToInterface(this->ip,m.getString());
    }
    return false;
}

bool UIConnection::sendChangeRole(Message::leader_update update) {
    if(this->ip != "") {
        Message m;
        m.setSender(myNode);
        m.setType((Message::Type)1);
        m.setCommand((Message::Command)0);
        m.setArgument((Message::Argument)this->session);

        m.setData(update);
        m.buildString();
        return sendToInterface(this->ip,m.getString());
    }
    return false;
}