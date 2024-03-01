#include "cache.hpp"
#include "response.hpp"
#include "request.hpp"
#include "util.hpp"
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip> 
#include <string>

pthread_mutex_t mutex_cache = PTHREAD_MUTEX_INITIALIZER;

Response * LRUCache::get(string url, int * cache_res){
    std::lock_guard<std::mutex> lock(mutex_cache);
    auto it = cacheMap.find(url);
    if (it!= cacheMap.end()) {
        if (it->second->response.cache_mode == CACHE_IMMUTABLE){
            cacheList.splice(cacheList.begin(), cacheList, it->second);
            *cache_res = CACHE_EXIST_VALID;
            return &(it->second->response);
        }
        else if (it->second->response.cache_mode == CACHE_MUST_REVALIDATE) {
            *cache_res = CACHE_EXIST_NONVALID;
            return &(it->second->response);
        }

        //check time
        if (isCacheExpired(it->second->response)){
            *cache_res = CACHE_EXIST_EXPIRED;
            return &(it->second->response);
        }

        //LRU and return response
        cacheList.splice(cacheList.begin(), cacheList, it->second);
        *cache_res = CACHE_EXIST_VALID;
        return &(it->second->response);
    }
    *cache_res = CACHE_NOEXIST;
    return NULL;
}

void LRUCache::put(string url, Response response, std::unique_ptr<Log>& log){
    std::lock_guard<std::mutex> lock(mutex_cache);
    if (response.cache_mode != CACHE_NO_STORE) {
        auto it = cacheMap.find(url);
        //update the cache
        if (it != cacheMap.end()){
            it -> second->response = response;
            cacheList.splice(cacheList.begin(), cacheList, it->second);
            return;
        }
        // if the list is full, delete the tail
        if(cacheList.size() == capacity){
            auto last = cacheList.back().ip;
            auto it = cacheMap.find(url);
            std::string msg = "evicted " + it->second->response.responseLine + " from cache";
            log->logNote(-1, msg);
            cacheList.pop_back();
            cacheMap.erase(last);
        }

        // insert a new item to the head
        cacheList.push_front(CacheItem{url, response});
        cacheMap[url] = cacheList.begin();
    }
}

bool LRUCache::isCacheExpired(Response response) { 
    if (response.expireTime.empty()) {
        return true;
    }

    std::tm tm = {};
    std::istringstream ss(response.expireTime);
    ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT"); 
    
    auto time_c = std::mktime(&tm);
    std::chrono::system_clock::time_point expireTime_chrono = std::chrono::system_clock::from_time_t(time_c);
    auto now = std::chrono::system_clock::now();

    if (now > expireTime_chrono){
        return true;
    }

    return false;
}