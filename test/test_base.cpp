#include "tornado/base/shared_stl.hpp"
#include <iostream>
using namespace boostshm;

void test_vector()
{
    const char* MyMemName ="MySharedMemory";

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

        for(auto iter=pVec->begin(); iter != pVec->end(); ++iter)
        {
            std::cout << *iter << std::endl;
        } 
    }
}


//void test_set()
//{
//    const char* MyMemName ="MySharedMemory";
//
//    ShmRemove  remove(MyMemName);
//    StringShmSet  shmset(MyMemName, 1024 * 100);
//
//    char buff[128];
//    for (int i = 0; i< 10 ; ++i)
//    {
//        int end = snprintf(buff, sizeof(buff)-1, "set_%d", i);
//        buff[end] = 0;
//        shmset.insert(buff, end);
//    }
//    
//    for (int i = 0; i< 10 ; ++i)
//    {
//        int end = snprintf(buff, sizeof(buff)-1, "set_set_%d", i);
//        buff[end] = 0;
//        shmset.insert(buff, end);
//    }
//    for (int i = 0; i< 10 ; ++i)
//    {
//        int end = snprintf(buff, sizeof(buff)-1, "set_%d", i);
//        buff[end] = 0;
//        shmset.insert(buff, end);
//    }
//   
//    //get
//    {
//        StringShmSet  shmset2(MyMemName, 1024 * 100);
//        auto  pSet = shmset2.get_set();
//
//        for(auto iter=pSet->begin(); iter != pSet->end(); ++iter)
//        {
//            std::cout << *iter << std::endl;
//        } 
//    }
//}


void test_map()
{
    typedef StringShmMap<int>  Mymap;

    const char* MyMemName ="MySharedMemory";

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

        for(auto iter=pMap->begin(); iter != pMap->end(); ++iter)
        {
            std::cout << iter->first << "  " <<iter->second << std::endl;
        } 
    }
}

void test_multimap()
{  
    typedef boostshm::ShmMultiMap<int, time_t> Mymap;

    const char* MyMemName ="MySharedMemory";

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

        for(auto iter=pMap->begin(); iter != pMap->end(); ++iter)
        {
            std::cout << iter->first << "  " <<iter->second << std::endl;
        } 

        auto end = pMap->lower_bound(99);
        for(auto iter=pMap->begin(); iter != end; ++iter)
        {
            std::cout << iter->first << "  " <<iter->second << std::endl;
        } 
      
        pMap->erase(pMap->begin(), end);

        for(auto iter=pMap->begin(); iter != pMap->end(); ++iter)
        {
            std::cout << iter->first << "  " <<iter->second << std::endl;
        } 

    }
}
  
int main()
{ 
    test_vector();
    //test_set();
    test_map();
    test_multimap();
    return 0;
}