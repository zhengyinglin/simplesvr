#include "tornado/shm/container.hpp"
#include "tornado/shm/mq.hpp"
#include <iostream>
using namespace boostshm;

void test_vector()
{
    const char* MyMemName ="MySharedMemory_VECTOR";

    ShmRemove  remove(MyMemName);
    StringShmVector  shmvec(MyMemName, 1024 * 100);

    char buff[128];
    for (int i = 0; i< 10 ; ++i)
    {
        int end = snprintf(buff, sizeof(buff)-1, "vector_%d", i);
        buff[end] = 0;
        shmvec.push_back(buff, end);
    }
    
    for (int i = 0; i< 10 ; ++i)
    {
        int end = snprintf(buff, sizeof(buff)-1, "vector_vector_%d", i);
        buff[end] = 0;
        shmvec.push_back(buff, end);
    }
   
    //get
    {
        StringShmVector  shmvec(MyMemName, 1024 * 100);
        auto  pVec = shmvec.get_vecotr();

        for(auto& item : *pVec )
        {
            std::cout << item << std::endl;
        } 
    }
}


void test_set()
{
    const char* MyMemName ="MySharedMemory_SET";

    ShmRemove  remove(MyMemName);
    StringShmSet  shmset(MyMemName, 1024 * 100);

    char buff[128];
    for (int i = 0; i< 10 ; ++i)
    {
        int end = snprintf(buff, sizeof(buff)-1, "set_%d", i);
        buff[end] = 0;
        shmset.insert(buff, end);
    }
    
    for (int i = 0; i< 10 ; ++i)
    {
        int end = snprintf(buff, sizeof(buff)-1, "set_set_%d", i);
        buff[end] = 0;
        shmset.insert(buff, end);
    }
    for (int i = 0; i< 10 ; ++i)
    {
        int end = snprintf(buff, sizeof(buff)-1, "set_%d", i);
        buff[end] = 0;
        shmset.insert(buff, end);
    }
   
    //get
    {
        StringShmSet  shmset2(MyMemName, 1024 * 100);
        auto  pSet = shmset2.get_set();
        for(auto& item : *pSet )
        {
            std::cout << item << std::endl;
        } 
    }
}


void test_map()
{
    typedef StringShmMap<int>  Mymap;

    const char* MyMemName ="MySharedMemory_MAP";

    ShmRemove  remove(MyMemName);
    Mymap  shmmap(MyMemName, 1024 * 100);

    char buff[128];
    for (int i = 0; i< 10 ; ++i)
    {
        int end = snprintf(buff, sizeof(buff)-1, "map_%d", i);
        buff[end] = 0;
        shmmap.insert_or_update(i, buff, end);
    }
    
    for (int i = 0; i< 10 ; ++i)
    {
        int end = snprintf(buff, sizeof(buff)-1, "map_map_%d", i);
        buff[end] = 0;
        shmmap.insert_or_update(i, buff, end);
    }
   

    //get
    {
        Mymap  shmmap(MyMemName, 1024 * 100);
        Mymap::Map*  pMap = shmmap.get_map();
        for(auto& item : *pMap )
        {
            std::cout << item.first << "  " <<item.second << std::endl;
        } 
    }
}

void test_map2()
{
    typedef ShmMap<int, int>  Intmap;
    const char* MyMemName ="MySharedMemory_MAP_INT";

    ShmRemove  remove(MyMemName);
    Intmap  shmmap(MyMemName, 1024 * 100);
    Intmap::Map*  pMap = shmmap.get_map();
    for (int i = 0; i< 10 ; ++i)
    {
        (*pMap)[i] = i * 100;
    }

    //get
    {
        Intmap  shmmap(MyMemName, 1024 * 100);
        Intmap::Map*  pMap = shmmap.get_map();
        for(auto& item : *pMap )
        {
            std::cout << item.first << "  " <<item.second << std::endl;
        } 
    }
}


//1字节 可以少内存
#pragma pack(push, 1)
struct STMapValue
{
public:
    STMapValue(int aa=0, short bb=0, long cc=0):
        a(aa), b(bb), c(cc)
    {
    }
    std::string value()
    {
        char buff[1024]={0};
        snprintf(buff, sizeof(buff), "a=%d, b=%d, c=%lu", a, b, c);
        return buff;
    }
    int a;
    short b;
    long  c;
};
#pragma pack(pop)

void test_map3()
{
    typedef ShmMap<int, STMapValue>  Structmap;
    const char* MyMemName ="MySharedMemory_MAP_STRUCT";

    ShmRemove  remove(MyMemName);
    Structmap  shmmap(MyMemName, 1024 * 100);
    Structmap::Map*  pMap = shmmap.get_map();
    for (int i = 0; i< 10 ; ++i)
    {
        STMapValue val(i, i, i);
        (*pMap)[i] = val;
    }

    //get
    {
        Structmap  shmmap(MyMemName, 1024 * 100);
        Structmap::Map*  pMap = shmmap.get_map();
        for(auto& item : *pMap )
        {
            std::cout << item.first << "  " << item.second.value() << std::endl;
        } 
    }
}

void test_multimap()
{  
    typedef boostshm::ShmMultiMap<int, time_t> Mymap;

    const char* MyMemName ="MySharedMemory_MULMAP";

    ShmRemove  remove(MyMemName);
    Mymap  shmmap(MyMemName, 1024 * 100);

    for (int i = 0; i< 10 ; ++i)
    {
        shmmap.insert(i, time(NULL));
    }
    
    for (int i = 0; i< 10 ; ++i)
    {
        shmmap.insert(i, time(NULL) + 1);
    }
    
    for (int i = 0; i< 10 ; ++i)
    {      
        shmmap.insert(i + 1000, time(NULL));
    }

    //get
    {
        Mymap  shmmap(MyMemName, 1024 * 100);
        Mymap::MultiMap*  pMap = shmmap.get_map();

        for(auto& item : *pMap )
        {
            std::cout << item.first << "  " <<item.second << std::endl;
        }

        auto end = pMap->lower_bound(99);
        for(auto iter=pMap->begin(); iter != end; ++iter)
        {
            std::cout << iter->first << "  " <<iter->second << std::endl;
        } 
      
        pMap->erase(pMap->begin(), end);

        for(auto& item : *pMap )
        {
            std::cout << item.first << "  " <<item.second << std::endl;
        }

    }
}

void test_mq()
{
    const char* MyMemName ="Test_MQ";

    ShmRemove  remove(MyMemName);
    MsgQueue   mq(MyMemName, 10, 128, true);

    char buff[128];
    for (int i = 0; i< 11 ; ++i)
    {
        int end = snprintf(buff, sizeof(buff)-1, "msg_%d", i);
        buff[end] = 0;
        bool ret = mq.send(buff, end);
        std::cout << "send buff=" << buff << ", ret= " <<  ret << std::endl;
    }
    
   
    //get
    {
       MsgQueue   mq(MyMemName, 10, 128, false);
       for (int i = 0; i< 11 ; ++i)
       {
            size_t  len = sizeof( buff );
            bool ret = mq.receive(buff, len);
            std::cout << "receive ret="<<  ret << ", buff="   << buff << std::endl;
        }
    }
}

void test_timerqueue()
{
    const char* MyMemName ="Test_TimerQueue";
    const int size = 1024*100;
    //ShmRemove  remove(MyMemName);
    {
        TimerQueue q(MyMemName, size);
        std::cout << q.erase(0) << std::endl;
        std::cout << q.erase(6) << std::endl;
    }
    uint32_t id = 0;
    {
         TimerQueue q(MyMemName, size);
         id = q.add(1000000);
         std::cout << id << std::endl;
    }

    {
         TimerQueue q(MyMemName, size);
         std::cout << q.erase(id) << std::endl;
    }
}




int main()
{ 
    /*test_vector();
    test_set();
    test_map();
    test_map2();
    test_map3();
    test_multimap();
    test_mq();*/
    test_timerqueue();
    return 0;
}