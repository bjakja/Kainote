#ifndef _AVS_MAPPED_LIST
#define _AVS_MAPPED_LIST

#include <list>
#include <unordered_map>

template <typename T>
class mapped_list
{
private:

  typedef std::list<T> ListType;
  std::list<T> list;
  std::unordered_map<T, typename ListType::iterator> map;

public:

  typedef typename ListType::iterator iterator;

  void push_back(const T& elem)
  {
    iterator it = list.insert(list.end(), elem);
    map.emplace(elem, it);
  }

  void push_front(const T& elem)
  {
    iterator it = list.insert(list.begin(), elem);
    map.emplace(elem, it);
  }

  bool empty() const
  {
    return list.empty();
  }

  size_t size() const
  {
    return list.size();
  }

  void remove(const T& elem)
  {
    auto map_it = map.find(elem);
    assert(map_it != map.end());

    iterator list_it = map_it->second;
    map.erase(map_it);
    list.erase(list_it);
  }

  void move_to_back(const T& elem)
  {
    auto map_it = map.find(elem);
    assert(map_it != map.end());

    iterator list_it = map_it->second;
    list.splice(list.end(), list, list_it);
  }

  iterator begin()
  {
    return list.begin();
  }

  iterator end()
  {
    return list.end();
  }
};

#endif // _AVS_MAPPED_LIST
