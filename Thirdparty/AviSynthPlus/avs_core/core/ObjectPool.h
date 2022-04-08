#ifndef _AVS_OBJECTPOOL_H
#define _AVS_OBJECTPOOL_H

#include <list>
#include <unordered_map>
#include <new>
#include <cassert>

template <typename T>
class ObjectPool
{
private:

  typedef std::list<char*> ListType;
  ListType UseList;
  ListType FreeList;

  typedef std::unordered_map<char*, typename ListType::iterator> MapType;
  MapType Map;

  static void DestructList(ListType* list, bool call_dtor)
  {
    const ListType::iterator end_it = list->end();
    if (call_dtor)
    {
      for (ListType::iterator it = list->begin(); it != end_it; ++it)
      {
        T* obj = (T*)(*it);
        obj->~T();
        delete [] (*it);
      }
    }
    else
    {
      for (ListType::iterator it = list->begin(); it != end_it; ++it)
      {
        delete [] (*it);
      }
    }
  }

  char * Alloc()
  {
    char* buff = NULL;
    if (!FreeList.empty())
    {
      buff = FreeList.front();
      UseList.splice(UseList.begin(), FreeList, FreeList.begin());
    }
    else
    {
      buff = new(std::nothrow) char[sizeof(T)];
      if (buff == NULL)
        return NULL;

      UseList.emplace_front(buff);
    }

    Map[buff] = UseList.begin();
    return buff;
  }

  void Free(char* obj)
  {
    MapType::iterator mit = Map.find(obj);
    assert(mit != Map.end());

    ListType::iterator lit = mit->second;
    assert(*lit == obj);

    FreeList.splice(FreeList.begin(), UseList, lit);

    // This line is actually not needed, but very cheap and results in a fully consistent map
    mit->second = FreeList.begin(); // Needed because MSVC violates spec and invalidates old list iterator
  }

public:


  // For the unfortunate lack of variadic templates in MSVC++2012
  T* Construct()
  {
    char* buff = Alloc();
    if (buff == NULL)
      throw std::bad_alloc();

    try
    {
      return new(buff) T();
    }
    catch(...)
    {
      Free(buff);
      throw;
    }
  }

  // For the unfortunate lack of variadic templates in MSVC++2012
  template<typename K>
  T* Construct(const K& param0)
  {
    char* buff = Alloc();
    if (buff == NULL)
      throw std::bad_alloc();

    try
    {
      return new(buff) T(param0);
    }
    catch(...)
    {
      Free(buff);
      throw;
    }
  }

  void Destruct(T* obj)
  {
    obj->~T();
    Free((char*)obj);
  }

  ~ObjectPool()
  {
    DestructList(&FreeList, false);
    DestructList(&UseList, true);
  }
};

#endif // _AVS_OBJECTPOOL_H
