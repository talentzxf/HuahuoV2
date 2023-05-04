//#pragma once
//
//#include "Runtime/Threads/ReadWriteLock.h"
//#include "Runtime/Utilities/NonCopyable.h"
//#include "Runtime/GfxDevice/utilities/GfxGenericHash.h"
//#include "External/google/sparsehash/densehashtable.h"
//#include "Runtime/Misc/EndOfFrameCallback.h"
//#include <functional>
//#include <string.h>
//
//template<typename key_type>
//struct MemCmpEqualTo : public std::binary_function<key_type, key_type, bool>
//{
//    bool operator()(const key_type& left, const key_type& right) const
//    {
//        return memcmp(&left, &right, sizeof(key_type)) == 0;
//    }
//};
//
//namespace GfxDoubleCacheConcurrencyPolicy
//{
//    struct AllowConcurrentGet
//    {
//        typedef ReadWriteLock LockType;
//        typedef ReadWriteLock::AutoReadLock AutoReadLock;
//        typedef ReadWriteLock::AutoWriteLock AutoWriteLock;
//
//        // Map deletor when resizing the map. The default implementation does immediate delete
//        struct MapDeletor
//        {
//            template<typename MapType>
//            void operator()(const MemLabelId & memLabel, MapType *map) const
//            {
//                UNITY_DELETE(map, memLabel);
//            }
//        };
//    };
//
//    struct NoConcurrentGet
//    {
//        struct LockType
//        {};
//
//        struct AutoReadLock : NonCopyable
//        {
//            AutoReadLock(LockType& mutex) {}
//        };
//        struct AutoWriteLock : NonCopyable
//        {
//            AutoWriteLock(LockType& mutex) {}
//        };
//
//        struct MapDeletor
//        {
//            template<typename MapType>
//            void operator()(const MemLabelId & memLabel, MapType *map) const
//            {
//                UNITY_DELETE(map, memLabel);
//            }
//        };
//    };
//
//    // Lockless get against insertions and deletions, uses delayed delete when need to grow/shrink
//    // Erase actually never shrinks the dense_hash_map, it just sets a flag that shrink will be considered at next insertion.
//    struct LocklessGet
//    {
//        typedef Mutex LockType; // Mutex for writing
//
//        // Read lock is NOP
//        struct AutoReadLock : NonCopyable
//        {
//            AutoReadLock(LockType& mutex) {}
//        };
//
//        // Write lock is a normal mutex
//        typedef Mutex::AutoLock AutoWriteLock;
//
//        // Map deletor when resizing the map. Delays delete until end of the frame
//        struct MapDeletor
//        {
//            template<typename MapType>
//            void operator()(const MemLabelId & memLabel, MapType *map) const
//            {
//                DelayedDelete(map, memLabel, true);
//            }
//        };
//    };
//}
//
//// Default empty/deleted key generator, just does memset
//template<typename key_type>
//struct GfxDoubleCacheDefaultEmptyDeletedGenerator
//{
//    key_type GetEmptyKey()
//    {
//        key_type res;
//        memset(&res, 0xFE, sizeof(res));
//        return res;
//    }
//
//    key_type GetDeletedKey()
//    {
//        key_type res;
//        memset(&res, 0xFF, sizeof(res));
//        return res;
//    }
//};
//
//template<typename KeyType
//         , typename ValueType
//         , typename HashKey = GfxGenericHash<KeyType>
//         , typename EqualKey = std::equal_to<KeyType>
//         , typename LockPolicy = GfxDoubleCacheConcurrencyPolicy::LocklessGet
//         , typename EmptyDeletedGenerator = GfxDoubleCacheDefaultEmptyDeletedGenerator<KeyType>
//         , MemLabelIdentifier MemLableId = kMemGfxDeviceId
//>
//class GfxDoubleCache
//{
//public:
//    typedef KeyType key_type;
//    typedef ValueType value_type;
//    typedef EqualKey equal_key;
//
//private:
//    typedef std::pair<const KeyType, ValueType> PairType;
//    typedef ::stl_allocator<PairType, MemLableId> AllocType;
//
//    struct SelectKey
//    {
//        const KeyType& operator()(const PairType& p) const
//        {
//            return p.first;
//        }
//    };
//    typedef dense_hashtable<PairType, KeyType, HashKey, SelectKey, EqualKey, AllocType> Map;
//
//    typedef typename LockPolicy::AutoReadLock AutoReadLock;
//    typedef typename LockPolicy::AutoWriteLock AutoWriteLock;
//
//public:
//    GfxDoubleCache(MemLabelId memLabel = kMemGfxDevice, bool autoInit = true)
//        : m_MemLabel(memLabel)
//        , m_Map(0)
//    {
//        AssertMsg(memLabel.identifier == MemLableId, "'memLabel' must match the provided templated parameter 'MemLableId' ");
//        if (autoInit)
//            Init();
//    }
//
//    ~GfxDoubleCache()
//    {
//        Dispose();
//    }
//
//    void Init()
//    {
//        AutoWriteLock wlock(m_Lock);
//        if (m_Map == 0)
//        {
//            Map *map = UNITY_NEW(Map, m_MemLabel)();
//            atomic_store_explicit(&m_Map, (atomic_word)map, memory_order_release);
//            EmptyDeletedGenerator gen;
//            key_type key = gen.GetEmptyKey();
//
//            map->set_empty_key(PairType(key, ValueType(0)));
//
//            key = gen.GetDeletedKey();
//            map->set_deleted_key(PairType(key, ValueType(0)));
//        }
//    }
//
//    void Dispose()
//    {
//        AutoWriteLock wlock(m_Lock);
//        if (m_Map != 0)
//        {
//            Map *map = (Map*)atomic_load_explicit(&m_Map, memory_order_acquire);
//            DebugAssert(map->size() == 0);
//            UNITY_DELETE(map, m_MemLabel);
//            m_Map = 0;
//        }
//    }
//
//    // Thread safe, uses ReadWriteLock
//    // Get a cached value, or if not found, create a new one using the callback provided and return the new one.
//    template<typename CreateCallback>
//    const value_type& Get(const key_type& key, CreateCallback callback)
//    {
//        // TODO we do hashing twice here when inserting.
//        typename Map::const_iterator it;
//        {
//            AutoReadLock rlock(m_Lock);
//            Map *map = (Map*)atomic_load_explicit(&m_Map, memory_order_acquire);  // Read in the current map pointer so it won't change while we search and compare against end()
//            it = map->find(key);
//            if (it != map->end() && it->second != ValueType(0))
//                return it->second;
//        }
//
//        // Need to check again because of possible race
//        AutoWriteLock wlock(m_Lock);
//        Map *map = (Map*)atomic_load_explicit(&m_Map, memory_order_acquire);  // We know this won't change on the fly because of the writelock above
//
//        std::pair<typename Map::iterator, bool> findRes = map->find_or_insert_noresize(PairType(key, ValueType(0)));
//        // Check for failure first, recycle if needed
//        if (findRes.first == map->end())
//        {
//            std::pair<bool, size_t> newSize = map->would_resize(1);
//            DebugAssert(newSize.first);
//            // Create new map, copy contents over, get rid of the old one.
//            Map *oldMap = map;
//            Map *newMap = UNITY_NEW(Map, m_MemLabel)(*oldMap, newSize.second);
//
//            atomic_store_explicit(&m_Map, (atomic_word)newMap, memory_order_release);
//
//            typename LockPolicy::MapDeletor()(m_MemLabel, oldMap);
//            return newMap->insert(PairType(key, callback(key))).first->second;
//        }
//
//        if (findRes.second) // Only create if not already there
//            findRes.first->second = callback(key); // Not optimal because we're locked during the creation callback.
//        return findRes.first->second;
//    }
//
//    // Insert or update. Returns the inserted value
//    const value_type & Set(const key_type &key, const value_type &value)
//    {
//        AutoWriteLock wlock(m_Lock);
//        Map *map = (Map*)atomic_load_explicit(&m_Map, memory_order_acquire);  // We know this won't change on the fly because of the writelock above
//
//        std::pair<typename Map::iterator, bool> findRes = map->find_or_insert_noresize(PairType(key, value));
//        // Check for failure first, recycle if needed
//        if (findRes.first == map->end())
//        {
//            std::pair<bool, size_t> newSize = map->would_resize(1);
//            DebugAssert(newSize.first);
//            // Create new map, copy contents over, get rid of the old one.
//            Map *oldMap = map;
//            Map *newMap = UNITY_NEW(Map, m_MemLabel)(*oldMap, newSize.second);
//
//            atomic_store_explicit(&m_Map, (atomic_word)newMap, memory_order_release);
//
//            typename LockPolicy::MapDeletor()(m_MemLabel, oldMap);
//            return newMap->insert(PairType(key, value)).first->second;
//        }
//
//        if (findRes.second) // Update
//            findRes.first->second = value;
//        return findRes.first->second;
//    }
//
//    // Insert IF key not already present. Returns true when key/value was added to the collection.
//    bool Insert(const key_type &key, const value_type &value)
//    {
//        AutoWriteLock wlock(m_Lock);
//        Map *map = (Map*)atomic_load_explicit(&m_Map, memory_order_acquire);  // We know this won't change on the fly because of the writelock above
//
//        std::pair<typename Map::iterator, bool> findRes = map->find_or_insert_noresize(PairType(key, value));
//        // Check for failure first, recycle if needed
//        if (findRes.first == map->end())
//        {
//            std::pair<bool, size_t> newSize = map->would_resize(1);
//            DebugAssert(newSize.first);
//            // Create new map, copy contents over, get rid of the old one.
//            Map *oldMap = map;
//            Map *newMap = UNITY_NEW(Map, m_MemLabel)(*oldMap, newSize.second);
//
//            atomic_store_explicit(&m_Map, (atomic_word)newMap, memory_order_release);
//
//            typename LockPolicy::MapDeletor()(m_MemLabel, oldMap);
//            newMap->insert(PairType(key, value));
//            return true;
//        }
//        return findRes.second;
//    }
//
//    // Find an item by key, or return notFoundValue if not found
//    const value_type &Find(const key_type &key, const value_type &notFoundValue)
//    {
//        typename Map::const_iterator it;
//        AutoReadLock rlock(m_Lock);
//        Map *map = (Map*)atomic_load_explicit(&m_Map, memory_order_acquire);  // Read in the current map pointer so it won't change while we search and compare against end()
//        it = map->find(key);
//        if (it != map->end() && it->second != ValueType(0))
//            return it->second;
//        return notFoundValue;
//    }
//
//    template<typename DestroyCallback>
//    void Cleanup(DestroyCallback callback)
//    {
//        AutoWriteLock wlock(m_Lock);
//        if (m_Map != 0)
//        {
//            Map *map = (Map*)atomic_load_explicit(&m_Map, memory_order_acquire);
//            typename Map::iterator it = map->begin();
//            typename Map::iterator eit = map->end();
//            for (; it != eit; ++it)
//                callback(it->second);
//            map->clear();
//        }
//    }
//
//    void Clear()
//    {
//        AutoWriteLock wlock(m_Lock);
//        if (m_Map != 0)
//        {
//            Map *map = (Map*)atomic_load_explicit(&m_Map, memory_order_acquire);
//            map->clear();
//        }
//    }
//
//    template<typename Cond, typename DestroyCallback>
//    void EraseIf(Cond cond, DestroyCallback callback)
//    {
//        AutoWriteLock wlock(m_Lock);
//        Map *map = (Map*)atomic_load_explicit(&m_Map, memory_order_acquire);
//        typename Map::iterator it = map->begin();
//        while (it != map->end())
//        {
//            typename Map::iterator cur = it++;
//            if (cond(*cur))
//            {
//                callback(cur->second);
//                map->erase(cur);
//            }
//        }
//    }
//
//    void Erase(const key_type &key)
//    {
//        AutoWriteLock wlock(m_Lock);
//        Map *map = (Map*)atomic_load_explicit(&m_Map, memory_order_acquire);
//        // dense_hashtable never shrinks at erase, if needed it does so at next insert.
//        map->erase(key);
//    }
//
//    bool Empty() const
//    {
//        Map *map = (Map*)atomic_load_explicit(&m_Map, memory_order_acquire);
//        return map->empty();
//    }
//
//    size_t Size() const
//    {
//        Map *map = (Map*)atomic_load_explicit(&m_Map, memory_order_acquire);
//        return map->size();
//    }
//
//private:
//    atomic_word m_Map; // Really a Map *
//    typename LockPolicy::LockType m_Lock;
//    const MemLabelId m_MemLabel;
//};
