
#include "Resource.h"

#include "ResourceManager.h"

Resource::Resource(const std::string& path) : _path(path)
{

}

void Resource::release()
{
    ResourceManager::unref(type(), _path);
}
