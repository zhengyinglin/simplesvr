#coding=utf-8
import urllib2

headers = {
    'User-Agent':'Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.31 (KHTML, like Gecko) Chrome/26.0.1410.43 Safari/537.31',
    #'Host': 'img1a.hualvtu.com',
    'Proxy-Connection': 'keep-alive',
    'Cache-Control': 'max-age=0',
    'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
    'Accept-Encoding': 'gzip,deflate,sdch',
    'Accept-Language': 'zh-CN,zh;q=0.8',
    'Accept-Charset': 'GBK,utf-8;q=0.7,*;q=0.3',
    'Cookie': 'piao_city=221; RK=Fd1e4D1kP1; pgv_pvi=315188224; ptcz=4886f4eb0249d1b13516c9cad798f5dfd45e5fcc5c0922440ef196ff44eec611; pt2gguin=o0979762787; o_cookie=979762787; pgv_pvid=1373164052'
        
 }


url = 'http://10.12.16.139:8888/cgi-bin/happy_fight_two?cmd=1'

req = urllib2.Request(url=url , headers=headers)

string = urllib2.urlopen(req , timeout = 10).read()

print len(string)
print string





