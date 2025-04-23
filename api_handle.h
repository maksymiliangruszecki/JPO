#ifndef API_HANDLE_H
#define API_HANDLE_H

#include <curl/curl.h>
#include <iostream>
#include <json/json.h>
#include <string>
#include <QMainWindow>
#include <QMessageBox>

using namespace std;

bool isNetworkAvailable() {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }

    // Try to connect to a reliable server with a short timeout
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com");
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);       // HEAD request only (no body)
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);      // 3 second timeout
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3L); // 3 second connect timeout
    // Don't follow redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    // Create a no-op write function to avoid printing response to stdout
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                     [](void*, size_t size, size_t nmemb, void*) -> size_t { return size * nmemb; });
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return (res == CURLE_OK);
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    /*
    * userp - pointer to user data, we cast void ptr to string and append the content
    * to the userp - user data. Finally the number of bytes that we taken care of is returned
    */
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to perform CURL request
bool performCurlRequest(const string& url, string& response,CURLcode * errorCode = nullptr, bool * errorOccured = nullptr, QString * errorMessage = nullptr) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Failed to initialize CURL" << endl;
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        if(errorCode)
            *errorCode = res;
        if(errorMessage)
            *errorMessage = curl_easy_strerror(res);
        if(errorOccured)
            *errorOccured = true;
        return false;
    }

    return true;
}

// Function to parse JSON response
bool parseJsonResponse(const string& jsonResponse, Json::Value& parsedRoot) {
    Json::Reader reader;

    bool parsingSuccessful = reader.parse(jsonResponse, parsedRoot);

    if (!parsingSuccessful) {
        cerr << "Failed to parse JSON: " << endl;
        return false;
    }

    return true;
}

Json::Value apiRequest(QString & url,QString * getDataAsQString = nullptr,CURLcode * errorCode = nullptr, bool * errorOccured = nullptr, QString * errorMessage = nullptr) { //default request "https://api.gios.gov.pl/pjp-api/rest/station/findAll"

    setlocale(LC_CTYPE, "Polish"); // for Polish characters
    qDebug()<<"API call to: "<<url;
    string api_url = url.toStdString();
    string response;
    Json::Value root;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    if (performCurlRequest(api_url, response,errorCode,errorOccured,errorMessage)) {
        if (!parseJsonResponse(response, root)) {
            //throw exception("corrupted json");
        }
    }
    curl_global_cleanup();
    if(getDataAsQString!=nullptr)
        *getDataAsQString = QString(response.c_str());
    //qDebug()<<root.asString();
    return root;
}

/*
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

QString getRequest(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            //std::cerr << "Curl request failed: " << curl_easy_strerror(res) << std::endl;
            QMessageBox msgbox;
            msgbox.setText("Curl request failed");
            msgbox.exec();
        }
        curl_easy_cleanup(curl);
    }
    return response.c_str();
}
*/
#endif // API_HANDLE_H
