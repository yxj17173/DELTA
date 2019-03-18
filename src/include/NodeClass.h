#ifndef __NODECLASS_H__
#define __NODECLASS_H__

#include <string>
#include <map>

class NodeClass {
public:
    typedef std::string Key;
    typedef int Value;
    typedef std::map<Key, Value> NodeClassMap;

    inline Value get(const Key& key) {
        return _map.at(key);
    }

    inline Value insert(const Key& key) {
        // std::pair<NodeClassMap::iterator, bool> ret;
        return (_map.insert(std::pair<Key, Value>(key, _map.size()))).first->second;
    }//insert the Key and return Value for the inserted Key.

    inline bool find(const Key& key) {
        return _map.find(key) != _map.end();
    }

    inline size_t size(void) {
        return _map.size();
    }

//private:
    NodeClassMap _map;
};

#endif // !__NODECLASS_H__
