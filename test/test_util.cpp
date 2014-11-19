#include "tornado/util/comm.h"
#include "tornado/util/json.h"
#include "tornado/util/ini.h"
#include "tornado/util/xml.h"
#include <iostream>
#include <string>
using namespace util::comm;

std::string input ="'f;dfdfs@^^#^#^#&^#&^#&*@&*@234567890qwertyuiopasdfghjkl;'zxcvbnm,.v./v./cv;sd;";
std::string hexstr;
std::string unhexstr;

std::string hexstr2;
std::string unhexstr2;

void test_hex()
{
    std::cout << input << std::endl;
    hexstr = hex(input);
    std::cout << hexstr <<std::endl;
    unhexstr = unhex( hexstr );
    std::cout << unhexstr <<std::endl;
}

void test_hex_2()
{
    char buff[1024];
    size_t size = hex(input.c_str(), input.size(), buff, sizeof(buff) );
    buff[size] = 0;
    std::cout << buff << std::endl;
    hexstr2.assign(buff, size);
    
    char buff2[1024];
    size_t size2 = unhex(buff, size, buff2, sizeof(buff2));
    buff2[size2] = 0;
    std::cout << buff2 << std::endl;
    unhexstr2.assign(buff2, size2);   
}


void test_json()
{
    Json  json;
    std::string ss = "{ \"root\": 123, \"its\": [100, 3003, 5995, 77777] }";
    json.paserFromString(ss);    
    std::cout << json.get<int>("root", -1) << std::endl;
    std::vector<int> vals;
    json.getVec<int>("its", vals);
    for(size_t i=0; i<vals.size(); i++)
        std::cout << " ***  "<< vals[i] << std::endl;
    //<< json.getString("its.num8", "nothing") << std::endl;
    //json.paserFromFile("/home/ghostzheng/somelib/svr/test/build/test.json");

}

void test_ini()
{
    Ini  ini;
    ini.paserFromFile("/tmp/shengdan.ini");
    ini.print();
    std::cout << "====================================>>>>>\n";
    std::cout << "TT_FEEDS_API.HostNum = " << ini.get<int>("TT_FEEDS_API.HostNum", -1) << std::endl;
    std::cout << "General.xml_path = " << ini.get<std::string>("General.xml_path", "nothing ...") << std::endl;
    std::cout << "General.xml_path = " << ini.get<std::string>("General.xml_pathoo", "nothing ...") << std::endl;
    std::cout << "General.test_time = " << ini.get<std::string>("General.test_time", "nothing ...") << std::endl;
}

void test_xml()
{
    Xml  xml;
    xml.paserFromFile("/tmp/WC2.xml");
/*
    std::cout << "====================================>>>>>\n";
    std::cout << "TT_FEEDS_API.HostNum = " << ini.get<int>("TT_FEEDS_API.HostNum", -1) << std::endl;
    std::cout << "General.xml_path = " << ini.get<std::string>("General.xml_path", "nothing ...") << std::endl;
    std::cout << "General.xml_path = " << ini.get<std::string>("General.xml_pathoo", "nothing ...") << std::endl;
    std::cout << "General.test_time = " << ini.get<std::string>("General.test_time", "nothing ...") << std::endl;*/
}

int main()
{ 
    test_hex();
    test_hex_2();
    if(hexstr == hexstr2 && unhexstr == unhexstr2 && input == unhexstr)
    {
        std::cout << "================> OK \n";
    }
    else
    {
        std::cout << "----------------> check failed\n";
    }

    test_json();
    test_ini();
    test_xml();

    return 0;
}

