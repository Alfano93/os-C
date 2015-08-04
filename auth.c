#include "openrc.h"

static int newline_offset(const char *text){
    const char *newline = strchr(text, '\n');
    if(!newline)
        return strlen(text);
    else
        return (int)(newline - text);
}

struct write_result{
    char *data;
    int pos;
};

static size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream){
    struct write_result *result = (struct write_result *)stream;

    if(result->pos + size * nmemb >= BUFFER_SIZE - 1){
        fprintf(stderr, "error: buffer too small \n");
        return 0;
    }

    memcpy(result->data + result->pos, ptr, size * nmemb);
    result->pos += size * nmemb;

    return size * nmemb;
}

static char *request(const char *url){
    CURL *curl = NULL;
    CURLcode status;
    struct curl_slist *headers = NULL;
    char *data = NULL;
    long code;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(!curl){
        if(data)
            free(data);
        if(curl)
            curl_easy_cleanup(curl);
        if(headers)
            curl_slist_free_all(headers);
        curl_global_cleanup();
        return NULL;
    }

    data = malloc(BUFFER_SIZE);

    if(!data){
        if(data)
            free(data);
        if(curl)
            curl_easy_cleanup(curl);
        if(headers)
            curl_slist_free_all(headers);
        curl_global_cleanup();
        return NULL;
    }

    struct write_result write_result = {
        .data = data,
        .pos = 0
    };

    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* openstack headers */
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);

    status = curl_easy_perform(curl);
    
      if(status != 0){
        fprintf(stderr, "error: unable to request data from %s:\n",url);
        fprintf(stderr, "%s\n", curl_easy_strerror(status));

        if(data)
            free(data);
        if(curl)
            curl_easy_cleanup(curl);
        if(headers)
            curl_slist_free_all(headers);
        curl_global_cleanup();
        return NULL;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if(code != 200){
        fprintf(stderr, "error: server responded with code %ld\n", code);

        if(data)
            free(data);
        if(curl)
            curl_easy_cleanup(curl);
        if(headers)
            curl_slist_free_all(headers);
        curl_global_cleanup();
        return NULL;
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();

    /* zero-terminate the result */
    data[write_result.pos] = '\0'
    return data;
} 
