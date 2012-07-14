
#include "ResourceManager.h"

ResourceManager::ResourceMap ResourceManager::_resourceTypes;

void ResourceManager::shutdown()
{
    auto it = _resourceTypes.begin();
    while (it != _resourceTypes.end()) {
        auto it2 = it->second.begin();
        auto elem = it++;
        while (it2 != it->second.end()) {
            auto elem2 = it2++;
            if (elem2->second.refcount == 0) {
                DBG_LOG("Unloading %s:%s", elem->first, elem2->first.c_str());
                elem2->second.res->unload();
                elem->second.erase(elem2);
            }
        }
        if (elem->second.size() == 0)
            _resourceTypes.erase(elem);
    }
}

void ResourceManager::unref(const char * type, const std::string& path)
{
    auto& map = _resourceTypes[type];
    auto it = map.find(path);
    if (it != map.end())
        it->second.refcount--;
}
