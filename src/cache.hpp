#ifndef _CACHE_HPP_
#define _CACHE_HPP_

#include "util.hpp"
#include "response.hpp"
#include <list>
#include <unordered_map>
#include <memory>

using namespace std;

struct CacheItem{
    string ip;
    Response response;
};

//Least Recently Used
class LRUCache{
private: 
    size_t capacity;
    list<CacheItem> cacheList;
    unordered_map<string, list<CacheItem>::iterator> cacheMap;
    std::mutex mutex_cache;
public:
    LRUCache(size_t capacity) : capacity(capacity) {}
    Response * get(string ip, int * cache_res);
    void put(string ip, Response response, std::unique_ptr<Log>& log);
    bool isCacheExpired(Response response);
};

#endif