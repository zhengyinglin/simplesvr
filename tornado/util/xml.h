/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-11-21
*/

#ifndef _UTIL_COMM_XML_
#define _UTIL_COMM_XML_

#include <iostream>
#include <sstream>
#include "boost/property_tree/xml_parser.hpp"
#include "boost_ptree.h"


namespace util
{

namespace comm
{

class Xml : public BoostPtree
{
public:
    virtual int paserFromFile(const std::string& filename) override
    {
        try
        {
            boost::property_tree::xml_parser::read_xml(filename, pt_); //, boost::property_tree::xml_parser::trim_whitespace); 
        }
        catch(const boost::property_tree::xml_parser::xml_parser_error& e)
        {
            std::cerr << __func__ << e.what() << std::endl;
            return -1;
        }
        return 0;
    }

    int paserFromString(const std::string& xml_str)
    {
        std::stringstream ss;
        ss << xml_str;
        try
        {
            boost::property_tree::xml_parser::read_xml(ss, pt_); //, boost::property_tree::xml_parser::trim_whitespace); 
        }
        catch(const boost::property_tree::xml_parser::xml_parser_error& e)
        {
            std::cerr << __func__ << e.what() << std::endl;
            return -1;
        }
        return 0;
    }
};


}//namespace comm

}//namespace util

#endif //_UTIL_COMM_XML_
