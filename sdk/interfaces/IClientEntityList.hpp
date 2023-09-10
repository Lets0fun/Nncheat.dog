#pragma once

#include "../misc/IHandleEntity.hpp"

class IClientNetworkable;
class IClientEntity;

class IClientEntityList
{
public:
    virtual IClientNetworkable*   GetClientNetworkable(int entnum) = 0;
    virtual void*                 vtablepad0x1(void) = 0;
    virtual void*                 vtablepad0x2(void) = 0;
    virtual IClientEntity*        GetClientEntity(int entNum) = 0;
    virtual IClientEntity*        GetClientEntityFromHandle(CBaseHandle hEnt) = 0;
    virtual int                   NumberOfEntities(bool bIncludeNonNetworkable) = 0;
    virtual int                   GetHighestEntityIndex(void) = 0;
    virtual void                  SetMaxEntities(int maxEnts) = 0;
    virtual int                   GetMaxEntities() = 0;
};

// classes, definitions
#include <cstddef>

namespace VirtualMethod
{
    template <typename T, std::size_t Idx, typename ...Args>
    constexpr auto call(void* classBase, Args... args) noexcept
    {
        return ((*reinterpret_cast<T(__thiscall***)(void*, Args...)>(classBase))[Idx])(classBase, args...);
    }
}

#define VIRTUAL_METHOD(returnType, name, idx, args, argsRaw) \
constexpr auto name args noexcept \
{ \
    return VirtualMethod::call<returnType, idx>argsRaw; \
}

class NetworkStringTable {
public:
    VIRTUAL_METHOD(int, addString, 8, (bool isServer, const char* value, int length = -1, const void* userdata = nullptr), (this, isServer, value, length, userdata))
};

class NetworkStringTableContainer {
public:
    VIRTUAL_METHOD(NetworkStringTable*, findTable, 3, (const char* name), (this, name))
};