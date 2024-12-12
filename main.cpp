#include <iostream>
#include <curl/curl.h> // Library to handle HTTP requests
#include <string>
#include <sstream> // For constructing JSON body for API requests
#include <chrono>  // For using std::chrono

using namespace std;


// API Key and URL for the LLM (Use your own keys and URL if needed)
const string API_URL = "https://api-inference.huggingface.co/models/bigscience/bloom-3b";

// Callback function to handle the response data from the API request
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userData) {
    size_t totalSize = size * nmemb;
    userData->append((char*)contents, totalSize);
    return totalSize;
}

// Function to check if the model is loaded and ready
bool checkModelStatus() {
       CURL* curl;
    CURLcode res;
    string responseData;

    // Initialize cURL globally and create a cURL handle
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        // Set HTTP headers for the API request
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + API_KEY).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // Create JSON request body containing the user input
        string requestBody;
        requestBody = "{\"inputs\": \"You are a conversational chatbot you would be required to answer to all the messages that I write next. \
        I don't want you to increase the size I actually want you to give me a cohenerent response.\", \"parameters\": {\"max_new_tokens\": 50}}";

        // Set cURL options for the API request
        curl_easy_setopt(curl, CURLOPT_URL, API_URL.c_str()); // API endpoint URL
        curl_easy_setopt(curl, CURLOPT_POST, 1L); // Use HTTP POST method
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str()); // Attach request body
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); // Set headers
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); // Handle response
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData); // Store response in responseData

        // Disable SSL certificate verification
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);  // Disable peer certificate verification
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);  // Disable host verification

        // Perform the API request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "cURL request failed: " << curl_easy_strerror(res) << endl;
        }

        // Clean up cURL handle and headers
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    if (responseData.find("\"error\"") != string::npos && responseData.find("loading") != string::npos) {
        return false;
    }


    cout << "Checking model loaded, " << responseData << endl;
    return true;
}

// Function to send the user's input to the LLM via an API request and return the response
string callBloomAPI(const string& userInput) {
    CURL* curl;
    CURLcode res;
    string responseData;

    // Initialize cURL globally and create a cURL handle
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        // Set HTTP headers for the API request
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + API_KEY).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // Create JSON request body containing the user input
        stringstream requestBody;
        requestBody << "{\"inputs\": \"" << userInput << "\", \"parameters\": {\"max_new_tokens\": 50}}";
        string requestBodyStr = requestBody.str();

        // Set cURL options for the API request
        curl_easy_setopt(curl, CURLOPT_URL, API_URL.c_str()); // API endpoint URL
        curl_easy_setopt(curl, CURLOPT_POST, 1L); // Use HTTP POST method
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBodyStr.c_str()); // Attach request body
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); // Set headers
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); // Handle response
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData); // Store response in responseData

        // Disable SSL certificate verification
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);  // Disable peer certificate verification
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);  // Disable host verification

        // Perform the API request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "cURL request failed: " << curl_easy_strerror(res) << endl;
        }

        // Clean up cURL handle and headers
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    return responseData; // Return the response from the LLM
}

int main() {
    cout << "Welcome to the Bloom AI Chatbot!" << endl;

    // Continuously check if the model is loaded and ready every 5 seconds
    auto start = std::chrono::steady_clock::now();
    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start);

        // Check if 5 seconds have passed
        if (duration.count() >= 5) {
            if (checkModelStatus()) {
                cout << "\nModel is loaded and ready! You can now input your message." << endl;
                break;
            } else {
                cout << "Model loading..." << endl;
                start = std::chrono::steady_clock::now(); // Reset the timer
            }
        }
    }

    // Now that the model is loaded, ask for user input
    while (true) {
        cout << "\n> Enter your message (type 'exit' to quit): ";
        string userInput;
        getline(cin, userInput);

        // Check if the user wants to exit
        if (userInput == "exit") {
            break;
        }

        cout << "\nSending your message to Bloom AI..." << endl;

        // Call the Bloom AI API and get the response
        string apiResponse = callBloomAPI(userInput);

        // Display the LLM's response
        cout << "\nResponse from Bloom AI: " << endl;
        cout << apiResponse << endl;
    }

    cout << "\nThank you for using Bloom AI Chatbot. Goodbye!" << endl;

    return 0;
}
