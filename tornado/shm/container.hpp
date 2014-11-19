#ifndef BASE_SHARED_STL_HPP_
#define BASE_SHARED_STL_HPP_

#include "boost/interprocess/managed_shared_memory.hpp"
#include "boost/interprocess/allocators/allocator.hpp"
#include "boost/interprocess/containers/vector.hpp"
#include "boost/interprocess/containers/set.hpp"
#include "boost/interprocess/containers/map.hpp"
#include "boost/interprocess/containers/string.hpp"
#include <utility> //std::make_pair
#include <string>
#include <assert.h>

namespace  boostshm
{

typedef boost::interprocess::managed_shared_memory  ManagedSharedMemory;

typedef boost::interprocess::allocator<char, ManagedSharedMemory::segment_manager> CharAllocator;
typedef boost::interprocess::basic_string<char, std::char_traits<char>, CharAllocator> shm_string;

//in linux  boost shared mem local  is /dev/shm/     shm name

class ShmRemove
{
public:
    ShmRemove(const std::string& strMemName)
    : m_memName(strMemName)
    { 
        boost::interprocess::shared_memory_object::remove(strMemName.c_str()); 
    }

    ShmRemove(const char* pMemName)
    : m_memName(pMemName)
    { 
        boost::interprocess::shared_memory_object::remove(pMemName); 
    }

    virtual ~ShmRemove()
    {
        boost::interprocess::shared_memory_object::remove(m_memName.c_str()); 
    }

private:
    std::string  m_memName;
};



template<typename ValueType>
class  ShmVector
{
public:
    typedef boost::interprocess::allocator<ValueType, ManagedSharedMemory::segment_manager> ShmemAllocator;
    typedef boost::interprocess::vector<ValueType, ShmemAllocator> Vector;

    ShmVector(const char* pMemName, size_t  alloc_size = 0)
    {
        bool bNewCreate = false;
        m_pSegment = NULL;
        try
        {
            m_pSegment = new ManagedSharedMemory(boost::interprocess::open_only, pMemName);    
        }
        catch (const std::exception& e )
        {
            assert(alloc_size > 0);
            m_pSegment = new ManagedSharedMemory(boost::interprocess::create_only, pMemName, alloc_size);
            bNewCreate = true;
        }

        if(bNewCreate)
        {
            const ShmemAllocator alloc_inst(m_pSegment->get_segment_manager());
            m_pVector = m_pSegment->construct<Vector>("Vector")(alloc_inst);
        }
        else
        {
             m_pVector = m_pSegment->find<Vector>("Vector").first;
        }
        assert(m_pVector != NULL) ;

        //check size
        assert( alloc_size &&  m_pSegment->get_size() == alloc_size );
    }

    virtual ~ShmVector()
    {
        //should not delete m_pVector;
        delete m_pSegment;
    }

    inline Vector* get_vecotr()
    {
        return m_pVector;
    }

    inline ManagedSharedMemory* get_segment()
    {
        return m_pSegment;
    }

    void destroy()
    {
        m_pSegment->destroy<Vector>("Vector");  
    }

protected:
    ManagedSharedMemory* m_pSegment;
    Vector *m_pVector;
};


class StringShmVector :  public ShmVector<shm_string>
{
public:
    StringShmVector(const char* pMemName, size_t  alloc_size = 0):
        ShmVector<shm_string>(pMemName, alloc_size)
    {
    }

    void push_back(const char* buff, size_t size)
    { 
        CharAllocator charAloc(ShmVector<shm_string>::m_pSegment->get_segment_manager());
        shm_string  val(buff, size, charAloc);
        ShmVector<shm_string>::m_pVector->push_back(val);
    }
};


template<typename KeyType>
class  ShmSet
{
public:
    typedef boost::interprocess::allocator<KeyType, ManagedSharedMemory::segment_manager> ShmemAllocator;
    typedef boost::interprocess::set<KeyType, std::less<KeyType>, ShmemAllocator> Set;

    ShmSet(const char* pMemName, size_t  alloc_size = 0)
    {
        bool bNewCreate = false;
        m_pSegment = NULL;
        try
        {
            m_pSegment = new ManagedSharedMemory(boost::interprocess::open_only, pMemName);    
        }
        catch (const std::exception& e )
        {
            assert(alloc_size > 0);
            m_pSegment = new ManagedSharedMemory(boost::interprocess::create_only, pMemName, alloc_size);
            bNewCreate = true;
        }

        if(bNewCreate)
        {
            const ShmemAllocator alloc_inst(m_pSegment->get_segment_manager());
            m_pSet = m_pSegment->construct<Set>("Set")(std::less<KeyType>(), alloc_inst);
        }
        else
        {
             m_pSet = m_pSegment->find<Set>("Set").first;
        }
        assert(m_pSet != NULL) ;

        //check size
        assert( alloc_size &&  m_pSegment->get_size() == alloc_size );
    }

    virtual ~ShmSet()
    {
        //should not delete m_pSet;
        delete m_pSegment;
    }

    inline Set* get_set()
    {
        return m_pSet;
    }

    const KeyType* find(const KeyType& key)
    {
        auto it = m_pSet->find(key);
        if( it == m_pSet->end() )
        { 
            return NULL;
        }
        return  &(*it);
    }
    
    inline size_t erase(const KeyType& key)
    {
       return m_pSet->erase(key);
    }

    inline ManagedSharedMemory* get_segment()
    {
        return m_pSegment;
    }

    void destroy()
    {
        m_pSegment->destroy<Set>("Set");  
    }


protected:
    ManagedSharedMemory* m_pSegment;
    Set *m_pSet;
};


class StringShmSet :  public ShmSet<shm_string>
{
public:
    StringShmSet(const char* pMemName, size_t  alloc_size = 0):
        ShmSet<shm_string>(pMemName, alloc_size)
    {
    }

    void insert(const char* buff, size_t size)
    { 
        CharAllocator charAloc(ShmSet<shm_string>::m_pSegment->get_segment_manager());
        shm_string  new_val(buff, size, charAloc);
        ShmSet<shm_string>::m_pSet->insert(new_val);
    }
};



template<typename KeyType, typename MappedType>
class ShmMap
{
public:
    typedef std::pair<const KeyType, MappedType> ValueType;
    typedef boost::interprocess::allocator<ValueType, ManagedSharedMemory::segment_manager> ShmemAllocator;
    typedef boost::interprocess::map<KeyType, MappedType, std::less<KeyType>, ShmemAllocator> Map;

    ShmMap(const char* pMemName, size_t  alloc_size = 0)
    {
        bool bNewCreate = false;
        m_pSegment = NULL;
        try
        {
            m_pSegment = new ManagedSharedMemory(boost::interprocess::open_only, pMemName);  
        }
        catch (const std::exception& e )
        {
            assert(alloc_size > 0);
            m_pSegment = new ManagedSharedMemory(boost::interprocess::create_only, pMemName, alloc_size);
            bNewCreate = true;
        }

        if(bNewCreate)
        {
            const ShmemAllocator alloc_inst(m_pSegment->get_segment_manager());
            m_pMap = m_pSegment->construct<Map>("Map")(std::less<KeyType>(), alloc_inst);
        }
        else
        {
             m_pMap = m_pSegment->find<Map>("Map").first;
        }
        assert(m_pMap != NULL) ;
        //check size
        assert( alloc_size &&  m_pSegment->get_size() == alloc_size );
    }

    virtual ~ShmMap()
    {
        //should not delete m_pMap;
        delete m_pSegment;
    }

    
    inline MappedType* find(const KeyType& key)
    {
        auto it = m_pMap->find(key);
        if( it == m_pMap->end() )
        { 
            return NULL;
        }
        return  &(it->second);
    }
    
    inline size_t erase(const KeyType& key)
    {
       return m_pMap->erase(key);
    }


    inline Map* get_map()
    {
        return m_pMap;
    }

    inline ManagedSharedMemory* get_segment()
    {
        return m_pSegment;
    }

    void destroy()
    {
        m_pSegment->destroy<Map>("Map");  
    }

protected:
    ManagedSharedMemory* m_pSegment;
    Map *m_pMap;
};



template<typename KeyType>
class StringShmMap :  public ShmMap<KeyType, shm_string>
{
public:
    StringShmMap(const char* pMemName, size_t  alloc_size = 0):
        ShmMap<KeyType, shm_string>(pMemName, alloc_size)
    {
    }

    void insert_or_update(const KeyType& key, const char* buff, size_t size)
    {
        shm_string* val = ShmMap<KeyType, shm_string>::find(key);
        if( val == NULL )
        { 
            CharAllocator charAloc(ShmMap<KeyType, shm_string>::m_pSegment->get_segment_manager());
            shm_string  new_val(buff, size, charAloc);
            //ShmMap<KeyType, shm_string>::m_pMap->insert(std::make_pair<const KeyType, shm_string>(key, new_val));
            ShmMap<KeyType, shm_string>::m_pMap->insert(std::make_pair(key, new_val));
        }
        else
        {
           val->assign(buff, size);
        }
    }

};



template<typename KeyType, typename MappedType>
class ShmMultiMap
{
public:
    typedef std::pair<const KeyType, MappedType> ValueType;
    typedef boost::interprocess::allocator<ValueType, ManagedSharedMemory::segment_manager> ShmemAllocator;
    typedef boost::interprocess::multimap<KeyType, MappedType, std::less<KeyType>, ShmemAllocator> MultiMap;

    ShmMultiMap(const char* pMemName, size_t  alloc_size = 0)
    {
        bool bNewCreate = false;
        m_pSegment = NULL;
        try
        {
            m_pSegment = new ManagedSharedMemory(boost::interprocess::open_only, pMemName);  
        }
        catch (const std::exception& e )
        {
            assert(alloc_size > 0);
            m_pSegment = new ManagedSharedMemory(boost::interprocess::create_only, pMemName, alloc_size);
            bNewCreate = true;
        }

        if(bNewCreate)
        {
            const ShmemAllocator alloc_inst(m_pSegment->get_segment_manager());
            m_pMultiMap = m_pSegment->construct<MultiMap>("MultiMap")(std::less<KeyType>(), alloc_inst);
        }
        else
        {
             m_pMultiMap = m_pSegment->find<MultiMap>("MultiMap").first;
        }
        assert(m_pMultiMap != NULL) ;
        //check size
        assert( alloc_size &&  m_pSegment->get_size() == alloc_size );
    }

    virtual ~ShmMultiMap()
    {
        //should not delete m_pMap;
        delete m_pSegment;
    }
    
    inline void insert(const KeyType key, MappedType val)
    {
         m_pMultiMap->insert( std::make_pair(key, val) );
    }

    inline size_t erase(const KeyType& key)
    {
       return m_pMultiMap->erase(key);
    }

    inline MultiMap* get_map()
    {
        return m_pMultiMap;
    }

    inline ManagedSharedMemory* get_segment()
    {
        return m_pSegment;
    }

    void destroy()
    {
        m_pSegment->destroy<MultiMap>("MultiMap");  
    }

protected:
    ManagedSharedMemory* m_pSegment;
    MultiMap *m_pMultiMap;
};


} // namespace  boostshm

#endif //BASE_SHARED_STL_HPP_
