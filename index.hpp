#pragma once


#include <iostream>
#include <vector>
#include <string>
#include <ifstream>
#include "util.hpp"


//该文件用于处理索引相关内容
namespace cx_index
{
    //正排索引处理：
    struct DocInfo
    {
        std::string title;//用于处理文档的标题
        std::string content;//用于处理文档的内容
        std::string url;//用于处理文档url
        uint64_t doc_id;//表示文档ID
    };

    //倒排索引处理：
    struct InvertedELem
    {
        int doc_id;//文档ID
        std::string word;//文档内容
        int weight;//文档权重
    };

    class Index
    {
    public:
        Index()
        {

        }
        ~Index()
        {

        }

        //正排:根据doc_id查找文档内容
        DocInfo* GetForwardIndex(uint64_t doc_id)
        {
            if(doc_id>forward_index.size())
            {
                std::cerr<<"该文件ID错误"<<std::endl;
                return nullptr;
            }
            return &forward_index[doc_id];
        }

        //倒排:根据关键字string查找倒排拉链---InvertedList
        InvertedList* GetInvertedIndex(const std::string& word)
        {
            auto it=inverted_index.find(word);
            if(it==inverted_index.end())
            {
                std::cerr<<"无法找到其:InvertedList "<<std::endl;
                return nullptr;
            }
            return &(it->second);
        }

        //构建索引
        bool BuildIndex(const string& input)//input即/data/raw_html/raw.txt文件
        {   
            std::ifstream in(input,std::ios::in|std::ios::binary);
            //检查是否打开
            if(!in.is_open())
            {
                std::cerr<<"file: "<<output<<" open failed"<<std::endl;
                return false;
            }
            //根据title+content+url \n 按行读取内容---getline
            std::string line;
            while(std::getline(in,line))
            {
                //构建正排索引
                DocInfo* doc = BuildForwardIndex(line);
                //检查：
                if(doc==nullptr)
                {
                    std::cerr<<"正排索引构建失败 "<<line<<std::endl;
                    continue;
                }

                //利用doc构建倒排索引
                BuildInvertedIndex(*doc);
                //构建失败

            }
            //关闭文件
            in.close();

            return true;
        }

    private:
        DocInfo* BuildForwardIndex(const std::string& line)
        {
            //1.解析line，将字符串切分
            std::vector<std::string> results;
            const std::string sep="\3";
            ns_util::StringUtil::CutString(line,&results,sep);
            //检查：
            if(results.size()!=3)
            {
                //该line解析失败
                return nullptr;
            }

            //2.将解析的字符串填入DocInfo中
            DocInfo doc;
            doc.title=results[0];
            doc.content=results[1];
            doc.url=results[2];
            doc.doc_id=forward_index.size();//此时vector中元素个数即为最新插入的id号，下标=id-1

            //3.将DocInfo中字符串放入vector<DocInfo>中
            forward_index.push_back(doc);
            
            return &forward_index.back();
        }
        bool BuildInvertedIndex(const DocInfo& doc)
        {
            return true;
        }


        //当是正排时，利用vector下标就是ID的方便特性，所以数据结构用vector数组
        std::vector<DocInfo> forward_index;
        //当时倒排时，通过关键字找对应的ID等信息(可能存在一对多的关系),此时利用unordered_map来解决(hash),注意second可能存在多个，要利用vector存放
typedef std::vector<InvertedELem> InvertedList;      
        std::unordered_map<std::string,InvertedList> inverted_index;
    };
}
