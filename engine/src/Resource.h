
#ifndef ENGINE_RESOURCE_H
#define ENGINE_RESOURCE_H

#include <string>

#define REGISTER_RESOURCE(cls)                         \
    cls (const std::string& path) : Resource(path) { } \
    const char * type() { return #cls; }               \
    static const char * className() { return #cls; }

class Resource
{
public:

    Resource(const std::string& path);
    virtual ~Resource() { }

    void release();

    inline const std::string& path() const {
        return _path;
    }

    virtual void load() = 0;
    virtual void unload() = 0;
    virtual const char * type() = 0;

private:

    std::string _path;

};

#endif
