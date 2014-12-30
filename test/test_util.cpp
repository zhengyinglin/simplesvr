#include "tornado/util/comm.h"
#include "tornado/util/json.h"
#include "tornado/util/ini.h"
#include "tornado/util/xml.h"
#include "tornado/util/random.h"
#include "tornado/util/scople_guard.h"
#include <iostream>
#include <string>
#include <stdint.h>
#include <vector>
#include <map>

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

void test_base64()
{
    TimeSpan tmp;
    std::string base64;
    std::string str = "from nginx-1.6.0/src/core/ngx_string.c  -+===--****^^^!@#$%^*[][]';<>?";
    encode_base64(base64, str);
    std::cout << str.length() << " : " << str << std::endl;
    std::cout << base64 << std::endl;

    std::string destr;
    int ret = decode_base64(destr, base64);
    std::cout << ret << std::endl;
    std::cout <<destr.length() << " : " << destr << std::endl;
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
    std::string xml_str = " \
<root name=\"rootname\"> \
    <file name=\"debug.log\"/> \
    <modules type=\"internal\">  \
        <module>Finance_Internal</module> \
        <module>Admin_Internal</module>  \
        <module>HR_Internal</module>  \
    </modules> \
</root>";

    Xml  xml;
    xml.paserFromString(xml_str);
    //name = rootname
    std::cout << xml.get<std::string>("root.<xmlattr>.name", "empty....") << std::endl;
    //name = debug.log
    std::cout << xml.get<std::string>("root.file.<xmlattr>.name", "empty....") << std::endl;

    auto modules = xml.getChild("root.modules");
    if(modules)
    {
        for( auto& m : modules.get() )
        {
            if( m.first != "<xmlattr>" )
            {   //<module>Finance_Internal</module>
                std::cout << "root.modules = " << m.first << "  " << m.second.get_value<std::string>() << std::endl;
            }
        }
    }
}

void test_rand()
{
    //默认int
    std::cout << "[0, 10) --> " << Random::instance().rand(10) << std::endl;

   
    {
        uint32_t min = 100;
        uint32_t max = 10000;
        std::cout << "[0, "<< max << ") --> " << Random::instance().rand<uint32_t>(max) << std::endl;
        std::cout << "["<< min<<", "<< max << ") --> " << Random::instance().rand<uint32_t>(min, max) << std::endl;
    }

    {
        uint64_t min = 100;
        uint64_t max = (uint64_t)1<<62;
        std::cout << "[0, "<< max << ") --> " << Random::instance().rand<uint64_t>(max) << std::endl;
        std::cout << "["<< min<<", "<< max << ") --> " << Random::instance().rand<uint64_t>(min, max) << std::endl;
    }

    {
        std::cout << "{90.0, 200.0, 600.0, 10.0} --> " << Random::instance().weightRand({90.0, 200.0, 600.0, 10.0}) << std::endl;
        std::vector<double> v = {90.0, 200.0, 600.0, 10.0};
        std::cout << "{90.0, 200.0, 600.0, 10.0} --> " << Random::instance().weightRand(v.begin(), v.end()) << std::endl;
    }
  

    {
        WeightRand objRand(  {90.0, 200.0, 600.0, 10.0}  );
        std::cout << "{90, 300, 600, 10} -->" << std::endl;
        std::map<int, int> m;
        for(int n=0; n<10000; ++n) 
        {
            ++m[objRand.rand()];
        }
        for(auto& p : m) 
        {
            std::cout << p.first << " generated " << p.second << " times\n";
        }
    }

}

void test_scople_guard()
{
    std::vector<int> values;

    values.push_back( 1000 );

    auto&& guard = makeGuard(  [&]{ values.pop_back();std::cout << "make guard\n";  }   );

    bool succ = false;
    if(succ)
    {
        guard.dismiss();
    }
}

int main()
{ /*
    test_base64();
    test_hex();
    test_hex_2();
    if(hexstr == hexstr2 && unhexstr == unhexstr2 && input == unhexstr)
    {
        std::cout << "================> OK \n";
    }
    else
    {
        std::cout << "----------------> check failed\n";
    }*/

    test_json();
 /*   test_ini();
    test_xml();

    test_rand();
    test_scople_guard(); */

    return 0;
}

