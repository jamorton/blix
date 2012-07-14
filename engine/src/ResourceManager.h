
#ifndef ENGINE_RESOURCE_MANAGER_H
#define ENGINE_RESOURCE_MANAGER_H

#include <string>
#include <exception>

#include "common.h"
#include "Resource.h"

class ResourceManager
{
private:

    struct ResourceMapEntry
    {
        ResourceMapEntry() : refcount(0), res(NULL) { }
        uint refcount;
        Resource * res;
    };

    typedef hash_table<const char *, hash_table<std::string, ResourceMapEntry> > ResourceMap;

    // This maps a resource type (e.g. texture) to a mapping from path name to resource instance.
    // This is so a single path can be loaded into multiple different types of resources without
    // conflict
    static ResourceMap _resourceTypes;

public:

    static void shutdown();

    template <class R>
    static R * load(const std::string& path)
    {
        ResourceMapEntry& entry = _resourceTypes[R::className()][path];

        if (entry.res == NULL) {
            entry.res = new R(path);
            entry.res->load();
        }

        entry.refcount++;
        return (R *)entry.res;
    }

private:

    friend class Resource;
    static void unref(const char * type, const std::string& path);




};




#endif
