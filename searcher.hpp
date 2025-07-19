#pragma once

#include <mutex>
#include <algorithm>
#include <jsoncpp/json/json.h>
#include <iterator>
#include <unordered_map>
#include "index.hpp"
#include "util.hpp"

namespace ns_searcher
{
    //定义InvertedElemPrint，用于解决显示一个文档重复出现在用户浏览器上问题
    struct InvertedElemPrint
    {
        uint64_t doc_id;
        int weight;
        std::vector<std::string> words;
        InvertedElemPrint():doc_id(0), weight(0){}
    };

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
            //std::cout<<"获取单例成功"<<std::endl;
            //日志显示：
            LOG(NORMAL, "获取index单例成功...");
            //2.根据对象建立索引
            index->BuildIndex(input);//bool BuildIndex(const string& input)//input即/data/raw_html/raw.txt文件
            //std::cout<<"构建正排和倒排索引成功"<<std::endl;
            //日志：
            LOG(NORMAL, "建立正排和倒排索引成功...");
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
            //ns_index::InvertedList inverted_list_all;//定义一个存放用户所有搜索关键词的链表std::vector<InvertedELem>---InvertedList
            //解决文档重复问题，用unordered_map来解决
            std::vector<InvertedElemPrint> inverted_list_all;
            std::unordered_map<uint64_t, InvertedElemPrint> tokens_map;
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
                
                //存在问题！！！
                //现在来解决用户输入的关键词在查找文档时，出现显示多个同一文档问题
                //本质上是在同一文档出现多个用户搜索对应的关键词
                //利用unordered_map来记录文档ID来防止重复的多次出现
                for(auto& elem:*inverted_list)//注意inverted_list是指针
                {
                    auto& item=tokens_map[elem.doc_id];//存在就直接获取，否则unordered_map会创建
                    item.doc_id=elem.doc_id;
                    //同一文档权重相加即可;
                    item.weight=elem.weight;
                    item.words.push_back(elem.word);
                }
                for(const auto& item:tokens_map)
                {
                    inverted_list_all.push_back(std::move(item.second));//item.second=InvertedElemPrint
                }
                
            }

            //3.汇总各部分查找，按照weight不同降序排序
            std::sort(inverted_list_all.begin(),inverted_list_all.end(),[](const InvertedElemPrint& s1,const InvertedElemPrint& s2){
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
                elem["desc"]=GetDesc(doc->content,item.words[0]);//注意我们想要的是content的一部分内容
                elem["url"]=doc->url;
                //for debug:
                elem["id"]=(int)doc->doc_id;
                elem["weight"]=item.weight;

                root.append(elem);
            }   
            //调试时用 StyledWriter ，正式使用时用FastWriter 
            //Json::StyledWriter writer;
            Json::FastWriter writer;
            *json_string = writer.write(root);
        }

        //截取内容：部分content
        std::string GetDesc(const std::string& html_content,const std::string& word)
        {
            //我们这里假设要求截取word出现的前五十个字节和后100个字节，不够就截取到开头和结尾
            //1.在content中查找word首次出现的位置,需要注意的是我们的文档是大小写分明的，word是被我们全部转换为小写的
            //不能直接find
            //std::size_t pos=html_content.find(word);
            //利用search函数
            auto it=std::search(html_content.begin(),html_content.end(),word.begin(),word.end(),[](const int x,const int y){
                return (std::tolower(x)==std::tolower(y));
            });
            //检查：
            if(it==html_content.end())
            {
                return "None";
            }
            size_t pos=std::distance(html_content.begin(),it);
            //2.确定截取范围
            //先假设截取范围
            std::size_t start=0;
            std::size_t end=html_content.size()-1;
            std::size_t front=50;
            std::size_t back=100;
            //注意size_t是无符号位的，比较最好用加号
            if(pos > start+front)
                start=pos-front;
            if(pos+back<end)
                end=pos+back;

            //3.截取相应的字符串
            //先检查start与end位置是否正确
            if(start>=end)
                return "None2";
            return html_content.substr(start,end-start);
        }

    private:
        ns_index::Index* index;//供系统调用的索引
    };
}
