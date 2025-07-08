#include "json.hpp"
using json = nlohmann::json;

#include<iostream>
#include<vector>
#include<map>
#include<string>
using namespace std;

//json序列化格式1
string func01()
{
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhangsan";
    js["to"] = "lisi";
    js["msg"] = "hello";
    
    string sendBuf = js.dump();//将json数据对象序列化
    return sendBuf;
}
string func02()
{
    json js;
    js["id"] = {1, 2, 3, 4, 5};
    js["msg"]["zhangsan"] = "hollo china";
    js["msg"]["lisi"] = "hollo world";
    js["msg1"] = {{"zhangsan", "hello china"}, {"lisi", "hello world"}};
    //键名一致会合并
    //string sendBuf = js.dump();
    //cout<<sendBuf.c_str()<<endl;
    return js.dump();
}
string func03()
{
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    map<int, string> m;
    m.insert({1, "zhangsan"});
    m.insert({2, "lisi"});
    m.insert({3, "wangwu"});

    json js;
    js["list"] = v;
    js["path"] = m;
    return js.dump();
}
int main()
{
    //func01();
    //func02();
    //func03();

    /*
    string recvBuf = func01();
    json jsBuf = json::parse(recvBuf);
    cout<<jsBuf["msg"];
    */
   /*
    string recvBuf = func02();
    json jsBuf = json::parse(recvBuf);
    cout<<jsBuf["id"]<<endl;
    auto arr = jsBuf["id"];
    cout<<arr[2]<<endl;
    auto msgjs = jsBuf["msg"];
    cout<<msgjs["zhangsan"]<<endl;
   */
    string recvBuf = func03();
    json jsBuf = json::parse(recvBuf);
    vector<int> vec = jsBuf["list"];
    map<int, string> mymap = jsBuf["path"];
    for(auto &v : vec)
    {
        cout<<v<<" ";
    }
    cout<<endl;
    for(auto &p : mymap)
    {
        cout<<p.first<<" "<<p.second<<endl;
    }
    return 0;
}