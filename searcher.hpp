#pragma once

#include <mutex>
#include <algorithm>
#include <jsoncpp/json/json.h>
#include "index.hpp"
#include "util.hpp"



namespace ns_searcher
{
    class Searcher
    {
    public:
        Searcher()
        {}
        ~Searcher()
        {}
        //初始化搜索引擎
        void InitSearcher(const std::string& input)
        {
            //1.获取或创建Index对象
            index=ns_index::Index::GetInstance();//得到单例的唯一对象
            //查看;
            std::cout<<"获取单例成功"<<std::endl;
            //2.根据对象建立索引
            index->BuildIndex(input);//bool BuildIndex(const string& input)//input即/data/raw_html/raw.txt文件
            std::cout<<"构建正排和倒排索引成功"<<std::endl;
        
        }
        //根据用户传输的关键词来建立相应的索引，并返回给用户浏览器Json对象
        void Search(const std::string& query,std::string* json_string)
        {
            //query用户传输的搜索关键词
            //json_string返回给用户浏览器的搜索结果
            //步骤：
            //1.根据query关键词来用Searcher分词
            std::vector<std::string> words;//分词存放位置
            //利用cppjieba分词工具分词
            ns_util::JiebaUtil::CutString(query,&words);

            //2.根据各个分词，进行index查找
            //注意点：我们查找不区分大小写，所以先将words中词转变成小写
            ns_index::InvertedList inverted_list_all;//定义一个存放用户所有搜索关键词的链表std::vector<InvertedELem>---InvertedList
            for(std::string word:words)
            {
                boost::to_lower(word);
                ns_index::InvertedList* inverted_list =index->GetInvertedIndex(word);//根据关键词查找倒排拉链
                //检查是否查找到
                if(inverted_list==nullptr)
                {
                    continue;
                }
                //查找到倒排拉链，放入关键词链表中
                //inverted_list_all.push_back(inverted_list)--错误写法，插入的不是一个位置，而是一条拉链
                //利用void insert (iterator position, InputIterator first, InputIterator last)在pos位置将一个链表或其他数据结构一个节点一个节点的插入
                inverted_list_all.insert(inverted_list_all.end(),inverted_list->begin(),inverted_list->end());
                //存在问题！！！
            }

            //3.汇总各部分查找，按照weight不同降序排序
            std::sort(inverted_list_all.begin(),inverted_list_all.end(),[](const ns_index::InvertedELem& s1,const ns_index::InvertedELem& s2){
                return s1.weight>s2.weight;
            });

            //4.将排序的结果生成对应Json字符串返回给用户浏览器中
            //利用JsoncppAPI接口
            Json::Value root;
            for(auto& item:inverted_list_all)
            {
                //根据倒排建立正排
                ns_index::DocInfo* doc=index->GetForwardIndex(item.doc_id);
                //检查是否存在
                if(nullptr==doc)
                {
                    continue;
                }
                Json::Value elem;
                elem["title"]=doc->title;
                elem["desc"]=GetDesc(doc->content,item.word);//注意我们想要的是content的一部分内容
                elem["url"]=doc->url;
                root.append(elem);
            }            
            Json::StyledWriter writer;
            *json_string = writer.write(root);
        }
        //截取内容：部分content
        std::string GetDesc(const std::string& html_content,const std::string& word)
        {
            //我们这里假设要求截取word出现的前五十个字节和后100个字节，不够就截取到开头和结尾
            //1.在content中查找word首次出现的位置
            std::size_t pos=html_content.find(word);
            //检查：
            if(pos==std::string::npos)
            {
                return "None";
            }

            //2.确定截取范围
            //先假设截取范围
            std::size_t start=0;
            std::size_t end=html_content.size()-1;
            std::size_t front=50;
            std::size_t back=100;
            if(pos-front > start)
                start=pos-front;
            if(pos+back<end)
                end=pos+back;

            //3.截取相应的字符串
            //先检查start与end位置是否正确
            if(start>=end)
                return "None";
            return html_content.substr(start,end-start);
        }

    private:
        ns_index::Index* index;//供系统调用的索引
    };
}
