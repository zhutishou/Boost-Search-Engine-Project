#include <iostream>
#include "searcher.hpp"
#include <string>

const std::string input="data/raw_html/raw.txt";
int main()
{
    //定义一个搜索对象
    ns_searcher::Searcher* search=new ns_searcher::Searcher();
    search->InitSearcher(input);
    std::string query;//用户输入关键词
    std::string json_string;//输出给用户的json串
    while(true)//服务一直启动
    {
        std::cout<<"请搜索输入："<<std::endl;
        std::cin>>query;
        search->Search(query,&json_string);
        std::cout<<json_string<<std::endl;
    }


    return 0;
}

