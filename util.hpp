#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "inc/cppjieba/Jieba.hpp"
#include <boost/algorithm/string.hpp>


//该文件用于存放文件处理功能
namespace ns_util
{
    class FileUtil
    {
    public:
        //功能：从file_path中读取文件信息到out中
        static bool ReadFile(const std::string& file_path,std::string* out)
        {
            std::ifstream in(file_path,std::ios::in);//构造一个文件方法
            //判断文件是否打开：
            if(!in.is_open())
            {
                std::cerr<<file_path<<" open false"<<std::endl;
                return false;
            }
            //打开成功，按行读取
            std::string line;
            while(std::getline(in,line))//istream& getline (istream& is, string& str);
            {
                //按行读取，将in中内容读取到line中，注意：getline返回值是&，会指向其所指向的类型隐式类型转换为bool
                *out+=line;
            }
            //关闭文件
            in.close();
            return true;
        }

    };


    //用于切分字符串
    class StringUtil
    {
    public:
        static void CutString(const std::string& line,std::vector<std::string>* out,const std::string& sep)
        {
            //利用boos库中split函数---boost::split(type, select_list, boost::is_any_of(","), boost::token_compress_on);
            //type---std::vector<std::string>，用于存放切割之后的字符串
            //select_list---select_list：传入的字符串，可以为空
            //boost::is_any_of("")---设定切割符为
            //boost::token_compress_on/off---将连续多个分隔符当一个，默认没有打开，当用的时候一般是要打开的
            //boost::split(*out,line,boost::is_any_of(sep),boost::token_compress_on); 
            boost::split(*out, line, boost::is_any_of(sep), boost::token_compress_on);
        }
    };

    //cppjiea
    const char* const DICT_PATH = "./dict/jieba.dict.utf8";
    const char* const HMM_PATH = "./dict/hmm_model.utf8";
    const char* const USER_DICT_PATH = "./dict/user.dict.utf8";
    const char* const IDF_PATH = "./dict/idf.utf8";
    const char* const STOP_WORD_PATH = "./dict/stop_words.utf8";
    class JiebaUtil
    {
    public:
        //注意：静态成员函数无法直接访问非静态变量和函数,无this指针--常量指针
        static void CutString(const std::string& src,std::vector<std::string>* out)
        {
            jieba.CutForSearch(src, *out);
        }
    private:
        static cppjieba::Jieba jieba;
    };
    cppjieba::Jieba jieba(DICT_PATH,HMM_PATH,USER_DICT_PATH,IDF_PATH,STOP_WORD_PATH);//静态成员变量在类外定义，属于所有类对象








}
