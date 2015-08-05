#include "openrc.c"

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

static char *POST_request(const char *url, const char *json_request){
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
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,json_request);

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
    data[write_result.pos] = '\0';
    return data;
}

int auth(){
    char *text;
    char url[URL_SIZE];
    char request[BUFFER_SIZE] ;

    json_t *root;
    json_error_t error;
    FILE * output_file;

    output_file = fopen("output.txt", "w+");

    snprintf(url, URL_SIZE, OS_AUTH_URL);
    strcat(url,"/tokens");

    strcat(request, "'{ auth: { tenantName: '");
    strcat(request, OS_TENANT_NAME);
    strcat(request, "', passwordCredentials: { username: '");
    strcat(request, OS_USERNAME);
    strcat(request, "' , password: '");
    strcat(request, OS_PASSWORD);
    strcat(request, "' }}}'");

    
    /*
    printf("url: %s \n request: %s \n",url,request); //DEBUG 
    */
    text = POST_request(url,request);
    if(!text)
        return 1;

    root = json_loads(text, 0, &error);

    if(!root){
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return 1;
    }

    printf("%s", json_dumps(root,JSON_INDENT(1)));

    return 0;
}

int main(){
    int auth_test = 0;
    auth_test = auth();
    return 0;
}  
