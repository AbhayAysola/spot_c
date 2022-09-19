#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "json-c/json.h"
#include "ap.h"

/* For dealing with spotify APs (access points) */

static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);

// Memory struct to store data from libcurl
struct Memory {
    char* response;
    size_t size;
};

/* Gets an AP from the AP resolve string
 * apresolve defaults to http://apresolve.spotify.com
 * Returns the AP specified by the ap_index
*/
char*
get_ap(char* apresolve, size_t ap_index) {
    // Initializing libcurl
    CURL* curl_handle;
    curl_global_init(CURL_GLOBAL_ALL);

    curl_handle = curl_easy_init();

    curl_easy_setopt(curl_handle, CURLOPT_URL, apresolve ? apresolve : "http://apresolve.spotify.com");

    // Make http request
    struct Memory chunk;
    chunk.response = malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk);

    CURLcode res = curl_easy_perform(curl_handle);

    // Request failure
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return NULL;
    }

    // Parse json data
    struct json_object* jobj;
    jobj = json_tokener_parse(chunk.response);
    struct json_object* ap_list = json_object_object_get(jobj, "ap_list");
    struct json_object* ap = json_object_array_get_idx(ap_list, ap_index);

    // libcurl cleanup
    curl_easy_cleanup(curl_handle);
    free(chunk.response);
    curl_global_cleanup();

    char* ap_string = malloc(30);
    strcpy(ap_string, json_object_to_json_string(ap)); // Copy from const char * to char *
    ap_string++;                             // Remove first and last character
    ap_string[strlen(ap_string) - 1] = '\0'; // because json returns string with quotes

    return ap_string;
}


static size_t
write_callback(char* data, size_t size, size_t nmemb, void* userdata) {
    size_t realsize = size * nmemb;
    struct Memory* mem = (struct Memory*)userdata;

    char* ptr = realloc(mem->response, mem->size + realsize + 1);
    if (ptr == NULL) {
        return 0;
    }

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize;
}