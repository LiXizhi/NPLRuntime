// Copyright Daniel Wallin 2009. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define LUABIND_BUILDING

#include <algorithm>
#include <limits>
#include <map>
#include <vector>
#include <luabind/typeid.hpp>
#include <luabind/detail/inheritance.hpp>

#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <mutex>
#endif

namespace luabind { namespace detail {

class_id const class_id_map::local_id_base =
    std::numeric_limits<class_id>::max() / 2;

namespace
{

  struct edge
  {
      edge(class_id target, cast_function cast) noexcept
        : target(target)
        , cast(cast)
      {}

      class_id target;
      cast_function cast;
  };

  inline bool operator<(edge const& x, edge const& y) noexcept
  {
      return x.target < y.target;
  }

  struct vertex
  {
      explicit vertex(class_id id) noexcept
        : id(id)
      {}

      class_id id;
      std::vector<edge> edges;
  };

  struct cache_record
  {
      class_id src;
      class_id target;
      class_id dynamic_id;
      std::ptrdiff_t object_offset;
      std::ptrdiff_t result_offset;
      int distance;
  };

  // Flat contiguous cache (replaces boost::tuple + std::map red-black tree)
  class cache
  {
  public:
      static std::ptrdiff_t const unknown;
      static std::ptrdiff_t const invalid;

      std::pair<std::ptrdiff_t, int> get(
          class_id src, class_id target, class_id dynamic_id
        , std::ptrdiff_t object_offset) const noexcept
      {
          for (std::size_t i = 0, n = m_records.size(); i < n; ++i)
          {
              cache_record const& r = m_records[i];
              if (r.src == src && r.target == target
                  && r.dynamic_id == dynamic_id && r.object_offset == object_offset)
              {
                  return std::pair<std::ptrdiff_t, int>(r.result_offset, r.distance);
              }
          }
          return std::pair<std::ptrdiff_t, int>(unknown, -1);
      }

      void put(
          class_id src, class_id target, class_id dynamic_id
        , std::ptrdiff_t object_offset
        , int distance, std::ptrdiff_t result_offset)
      {
          cache_record rec;
          rec.src = src;
          rec.target = target;
          rec.dynamic_id = dynamic_id;
          rec.object_offset = object_offset;
          rec.result_offset = result_offset;
          rec.distance = distance;
          m_records.push_back(rec);
      }

      void invalidate() noexcept
      {
          m_records.clear();
      }

  private:
      std::vector<cache_record> m_records;
  };

  std::ptrdiff_t const cache::unknown =
      std::numeric_limits<std::ptrdiff_t>::max();
  std::ptrdiff_t const cache::invalid = cache::unknown - 1;

  struct queue_entry
  {
      queue_entry(void* p, class_id vertex_id, int distance) noexcept
        : p(p)
        , vertex_id(vertex_id)
        , distance(distance)
      {}

      void* p;
      class_id vertex_id;
      int distance;
  };

} // namespace unnamed

class cast_graph::impl
{
public:
    std::pair<void*, int> cast(
        void* p, class_id src, class_id target
      , class_id dynamic_id, void const* dynamic_ptr) const;
    void insert(class_id src, class_id target, cast_function cast);

private:
    std::vector<vertex> m_vertices;
    mutable cache m_cache;
    mutable std::vector<queue_entry> m_bfs_queue;
    mutable std::vector<unsigned char> m_visited;
};

std::pair<void*, int> cast_graph::impl::cast(
    void* const p, class_id src, class_id target
  , class_id dynamic_id, void const* dynamic_ptr) const
{
    if (LUABIND_LIKELY(src == target))
        return std::pair<void*, int>(p, 0);

    if (src >= m_vertices.size() || target >= m_vertices.size())
        return std::pair<void*, int>((void*)0, -1);

    std::ptrdiff_t const object_offset =
        (char const*)dynamic_ptr - (char const*)p;

    std::pair<std::ptrdiff_t, int> cached = m_cache.get(src, target, dynamic_id, object_offset);

    if (cached.first != cache::unknown)
    {
        if (cached.first == cache::invalid)
            return std::pair<void*, int>((void*)0, -1);
        return std::pair<void*, int>((char*)p + cached.first, cached.second);
    }

    // Contiguous reusable vector BFS (eliminates std::deque + boost::dynamic_bitset allocations)
    m_bfs_queue.clear();
    m_bfs_queue.push_back(queue_entry(p, src, 0));

    m_visited.assign(m_vertices.size(), 0);

    for (std::size_t head = 0; head < m_bfs_queue.size(); ++head)
    {
        queue_entry const qe = m_bfs_queue[head];

        if (m_visited[qe.vertex_id])
            continue;
        m_visited[qe.vertex_id] = 1;

        vertex const& v = m_vertices[qe.vertex_id];

        if (v.id == target)
        {
            std::ptrdiff_t const res_offset = (char*)qe.p - (char*)p;
            m_cache.put(
                src, target, dynamic_id, object_offset
              , qe.distance, res_offset
            );

            return std::pair<void*, int>(qe.p, qe.distance);
        }

        for (std::size_t ei = 0, en = v.edges.size(); ei < en; ++ei)
        {
            edge const& e = v.edges[ei];
            if (m_visited[e.target])
                continue;
            if (void* casted = e.cast(qe.p))
                m_bfs_queue.push_back(queue_entry(casted, e.target, qe.distance + 1));
        }
    }

    m_cache.put(src, target, dynamic_id, object_offset, -1, cache::invalid);

    return std::pair<void*, int>((void*)0, -1);
}

void cast_graph::impl::insert(
    class_id src, class_id target, cast_function cast)
{
    class_id const max_id = std::max(src, target);

    if (max_id >= m_vertices.size())
    {
        m_vertices.reserve(max_id + 1);
        for (class_id i = m_vertices.size(); i < max_id + 1; ++i)
            m_vertices.push_back(vertex(i));
    }

    std::vector<edge>& edges = m_vertices[src].edges;

    std::vector<edge>::iterator i = std::lower_bound(
        edges.begin(), edges.end(), edge(target, 0)
    );

    if (i == edges.end() || i->target != target)
    {
        edges.insert(i, edge(target, cast));
        m_cache.invalidate();
    }
}

std::pair<void*, int> cast_graph::cast(
    void* p, class_id src, class_id target
  , class_id dynamic_id, void const* dynamic_ptr) const
{
    return m_impl->cast(p, src, target, dynamic_id, dynamic_ptr);
}

void cast_graph::insert(class_id src, class_id target, cast_function cast)
{
    m_impl->insert(src, target, cast);
}

cast_graph::cast_graph()
  : m_impl(new impl)
{}

cast_graph::~cast_graph()
{}

LUABIND_API class_id allocate_class_id(type_id const& cls)
{
    typedef std::map<type_id, class_id> map_type;

#ifndef EMSCRIPTEN_SINGLE_THREAD
    static std::mutex s_mutex;
    std::lock_guard<std::mutex> lock(s_mutex);
#endif
    static map_type registered;
    static class_id id = 0;

    std::pair<map_type::iterator, bool> inserted = registered.insert(
        std::make_pair(cls, id));

    if (inserted.second)
        ++id;

    return inserted.first->second;
}

}} // namespace luabind::detail
