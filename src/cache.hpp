#ifndef _CACHE_HPP_
#define _CACHE_HPP_

#include "util.hpp"
#include "response.hpp"
#include <list>
#include <unordered_map>

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
public:
    LRUCache(size_t capacity) : capacity(capacity) {}
    Response * get(string ip);
    //insert or update
    void put(string ip, Response response);
    bool isCacheExpired(string responseDate, int maxAge);
    bool validate(Response response);
};

#endif