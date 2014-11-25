/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-11-21
*/
#ifndef _UTIL_COMM_JSON_
#define _UTIL_COMM_JSON_

#include <iostream>
#include <sstream>
#include "boost/property_tree/json_parser.hpp"
#include "boost_ptree.h"

namespace util
{

namespace comm
{

class Json : public BoostPtree
{
public:
    virtual int paserFromFile(const std::string& filename) override
    {
        try
        {
            boost::property_tree::read_json(filename, pt_); 
        }
        catch(const boost::property_tree::json_parser_error& e)
        {
            std::cerr << __func__ << e.what() << std::endl;
            return -1;
        }
        return 0;
    }

    int paserFromString(const std::string& json_str)
    {
        std::stringstream ss;
        ss << json_str;
        try
        {
            boost::property_tree::read_json(ss, pt_); 
        }
        catch(const boost::property_tree::json_parser_error& e)
        {
            std::cerr << __func__ << e.what() << std::endl;
            return -1;
        }
        return 0;
    }
};


}//namespace comm

}//namespace util

#endif //_UTIL_COMM_JSON_