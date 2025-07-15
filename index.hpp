#pragma once


#include <iostream>
#include <vector>
#include <string>
#include <ifstream>
#include <unordered_map>
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
        //正排：
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
        //倒排：
        bool BuildInvertedIndex(const DocInfo& doc)
        {
            struct word_cnt
            {
                word_cnt():
                    title_cnt(0),content_cnt(0)
                {}
                int title_cnt;
                int content_cnt;
            };
            //1.分词：
            //对标题分词
            std::vector<std::string> title_words;
            ns_util::JiebaUtil::CutString(doc.title,&title_words);
            //对内容分词
            std::vector<std::string> content_words;
            ns_util::JiebaUtil::CutString(doc.content,&content_words);

            //2.词频统计：注意：不区分大小写，用boost::to_lower()
            std::unordered_map<std::string,word_cnt> word_map;
            //对标题进行统计：
            for(std::string e:title_words)
            {
                boost::to_lower(e);
                word_map[e].title_cnt++;
            }   
            //对内容进行统计：
            for(std::string e:content_words)
            {
                boost::to_lower(e);
                word_map[e].content_cnt++;
            }

            //3.相关性处理
            //在这里我们直接将title权重设置为10，content权重设置为1
#define X 10
#define Y 1
            for(auto& word_pair:word_map)//处理成pair类型
            {
                InvertedELem item;
                item.doc_id=doc.doc_id;//DocInfo
                item.word=word_pair.first;
                item.weight=word_pair.second.title_cnt*X+word_pair.second.content_cnt*Y;
                InvertedList& Inverted_list=inverted_index[word_pair.first];
                Inverted_list.push_back(std::move(item));
            }
            return true;
        }


        //当是正排时，利用vector下标就是ID的方便特性，所以数据结构用vector数组
        std::vector<DocInfo> forward_index;
        //当时倒排时，通过关键字找对应的ID等信息(可能存在一对多的关系),此时利用unordered_map来解决(hash),注意second可能存在多个，要利用vector存放
typedef std::vector<InvertedELem> InvertedList;      
        std::unordered_map<std::string,InvertedList> inverted_index;//first--存放词,second存放一条链表，表中是词为first的不同权重/doc_id的节点
    };
}
