
#include "cpp-httplib/httplib.h"
#include "searcher.hpp"

const std::string input="data/raw_html/raw.txt";
const std::string root_path="./wwwroot";

int main()
{
    //定义一个搜索引擎对象,并且初始化正向和逆向索引
    ns_searcher::Searcher search;
    search.InitSearcher(input);
    //定义一个Server服务器,并且查找路径设置为./wwwroot
    httplib::Server svr;
    svr.set_base_dir(root_path.c_str());
    svr.Get("/s",[&search](const httplib::Request& req,httplib::Response& rsp){
        //检查用户是否输入搜索关键词：
        if(!req.has_param("word"))
        {
            rsp.set_content("搜索时必须要写内容呢！！！","text/plain:charset=utf-8");
            return;
        }
        //接受用户输入的关键词;
        std::string word=req.get_param_value("word");
        //std::cout<<"用户在要搜索："<<word<<std::endl;
        //当我们设置了日志后就日志形式显示给后端：
        LOG(NORMAL, "用户搜索的是: " + word);
        //定义json串，并且在searcher中得到搜索内容返回json串
        std::string json_string;
        search.Search(word,&json_string);
        rsp.set_content(json_string,"application/json");

    });
    //显示服务器启动：
    LOG(NORMAL, "服务器启动成功...");
    svr.listen("0.0.0.0",8081);
    return 0;
}
