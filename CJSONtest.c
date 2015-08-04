#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <jansson.h>
#include <curl/curl.h>

#define BUFFER_SIZE    (256 * 1024) /* 256 KB */

#define URL_FORMAT    "https://api.github.com/repos/%s/%s/commits"
#define URL_SIZE      256

/* Return the offset of the first newline in text or the Length of 
   text if there's no new line character */

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
        fprintf(stderr, "error: too small buffer\n");
        return 0;
    }

    memcpy(result->data + result->pos, ptr, size *nmemb);
    result->pos += size * nmemb;

    return size * nmemb;
}

static char *request( const char *url){
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

    /* Github commits API v3 requires a User-Agent header */ 
    headers = curl_slist_append(headers, "User-Agent: Jansson-Tutorial");
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

int main(int argc, char *argv[]){

    size_t i;
    char *text;
    char url[URL_SIZE];

    json_t *root;
    json_error_t error;
    FILE * out_file;
    out_file = fopen("output.txt","w+");

    if(argc != 3){
        fprintf(stderr, "usage: %s USER REPOSITORY \n\n", argv[0]);
        fprintf(stderr, "List commits at USER's REPOSITORY. \n\n");
        return 2;
    }

    snprintf(url, URL_SIZE, URL_FORMAT, argv[1], argv[2]);
	
    text = request(url);
    if(!text)
        return 1;

    root = json_loads(text, 0, &error);

    if(json_dump_file(root, "output.txt", JSON_INDENT(1))){
        fprintf(stderr, "error: could not write to output file \n");
        return 1;
    }
    printf("%s",json_dumps(root,JSON_INDENT(1)));
    
    fclose(out_file);

    free(text);

    if(!root){
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return 1;
    }

    if(!json_is_array(root)){
        fprintf(stderr, "error: root is not an array\n");
        json_decref(root);
        return 1;
    }

    for(i = 0; i < json_array_size(root); i++){
        json_t *data, *sha, *commit, *message;
        const char *message_text;

        data = json_array_get(root, i);
        if(!json_is_object(data)){
            fprintf(stderr, "error: commit data %d is not an object\n", i + 1);
            json_decref(root);
            return 1;
        }

        sha = json_object_get(data, "sha");
        if(!json_is_string(sha)){
            fprintf(stderr, "error: commit %d: sha is not a string\n", i + 1);
            json_decref(root);
            return 1;
        }
		
        commit = json_object_get(data, "commit");
        if(!json_is_object(commit)){
            fprintf(stderr, "error: commit %d: commit is not an object\n", i + 1);
            json_decref(root);
            return 1;
	}

        message = json_object_get(commit, "message");
        if(!json_is_string(message)){
	    fprintf(stderr, "error: commit %d: message is not a string\n", i+1);
	    json_decref(root);
	    return 1;
	}

        message_text = json_string_value(message);
        printf("%.8s %.*s\n", 
            json_string_value(sha),
            newline_offset(message_text),
            message_text);
        }

        json_decref(root);
        return 0;
}

