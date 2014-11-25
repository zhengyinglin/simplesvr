/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-11-21
*/

#ifndef _UTIL_COMM_BOOST_PTREE_
#define _UTIL_COMM_BOOST_PTREE_


#include "boost/property_tree/ptree.hpp"
#include "boost/optional.hpp"
#include <vector>

namespace util
{

namespace comm
{

typedef boost::property_tree::ptree  PTree;

class BoostPtree
{
public:
    virtual int paserFromFile(const std::string& filename) = 0;

    template<class T> inline
    T get(const std::string& item_name, const T& default_val)
    { 
       return pt_.get<T>( item_name, default_val);
    }

    inline boost::optional< PTree& > getChild(const std::string& name)
    {
        return pt_.get_child_optional(name);
    }

    template<class T>
    void getVec(const std::string& item_name, std::vector<T>& vecItem)
    {
       auto child =  getChild(item_name); 
       if(!child)
       {
            return;
       }
       for(auto& v : child.get())
       {
           vecItem.push_back( v.second.get_value<T>() );
       }
    }

protected:
    PTree  pt_;
};


}//namespace comm

}//namespace util

#endif //_UTIL_COMM_BOOST_PTREE_