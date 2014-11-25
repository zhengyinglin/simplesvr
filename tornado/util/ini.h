/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-11-21
*/
#ifndef _UTIL_COMM_INI_
#define _UTIL_COMM_INI_

#include <iostream>
#include "boost/property_tree/ini_parser.hpp"
#include "boost_ptree.h"

namespace util
{

namespace comm
{

class Ini : public BoostPtree
{
public:
    virtual int paserFromFile(const std::string& filename) override
    {
        try
        {
            boost::property_tree::ini_parser::read_ini(filename, pt_); 
        }
        catch(const boost::property_tree::ini_parser::ini_parser_error& e)
        {
            std::cerr << __func__ << e.what() << std::endl;
            return -1;
        }
        return 0;
    }

    void print()
    {
        for(auto& item : pt_)
        {
            std::cout << '[' << item.first << "]\n";
            for(auto& value : item.second)
            {
                std::cout << value.first << "=" << value.second.get_value<std::string>() << "\n";
            }
        }
    }
};


}//namespace comm

}//namespace util

#endif //_UTIL_COMM_INI_
