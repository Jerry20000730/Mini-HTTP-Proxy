#include "cache.hpp"
#include "util.hpp"
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip> 
#include <string>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

Response * LRUCache::get(string ip){
    pthread_mutex_lock(&mutex);
    auto it = cacheMap.find(ip);
    if (it!= cacheMap.end()) {
        if (it->second->response.cache_mode == CACHE_IMMUTABLE){
            cacheList.splice(cacheList.begin(), cacheList, it->second);
            pthread_mutex_unlock(&mutex);
            return &(it->second->response);
        }
        else if (it->second->response.cache_mode == CACHE_MUST_REVALIDATE){
            pthread_mutex_unlock(&mutex);
            validate(it->second->response);
        }

        //check time
        if (isCacheExpired(it->second->response.date, it->second->response.max_age)){
            pthread_mutex_unlock(&mutex);
            return NULL;
        }

        //LRU and return response
        cacheList.splice(cacheList.begin(), cacheList, it->second);
        pthread_mutex_unlock(&mutex);
        return &(it->second->response);
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void LRUCache::put(string ip, Response response){
    pthread_mutex_lock(&mutex);
    if (response.cache_mode != CACHE_NO_STORE){
        auto it = cacheMap.find(ip);
        //update the cache
        if (it != cacheMap.end()){
            it -> second->response = response;
            cacheList.splice(cacheList.begin(), cacheList, it->second);
            pthread_mutex_unlock(&mutex);
            return;
        }
        //if the list is full, delete the tail
        if(cacheList.size() == capacity){
            auto last = cacheList.back().ip;
            cacheList.pop_back();
            cacheMap.erase(last);
        }

        //insert a new item to the head
        cacheList.push_front(CacheItem{ip, response});
        cacheMap[ip] = cacheList.begin();
    }
    pthread_mutex_unlock(&mutex);
}

bool LRUCache::isCacheExpired(string responseDate, int maxAge){
    pthread_mutex_lock(&mutex);
    std::tm tm = {};
    std::istringstream ss(responseDate);
    ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT"); 

    auto responseTime = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    auto expiryTime = responseTime + std::chrono::seconds(maxAge);
    auto now = std::chrono::system_clock::now();

    if (now>expiryTime){
        pthread_mutex_unlock(&mutex);
        return true;
    }
    pthread_mutex_unlock(&mutex);
    return false;
}

bool LRUCache::validate(Response response){
    pthread_mutex_lock(&mutex);
    if (response.Etag == ""){
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    if (response.status != 304){
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    pthread_mutex_unlock(&mutex);
    return false;
}

