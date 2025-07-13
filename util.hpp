#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>


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
        static void CutString(const std::string& line,std::vector<std::string>* out,const std::string7 sep)
        {
            //利用boos库中split函数---boost::split(type, select_list, boost::is_any_of(","), boost::token_compress_on);
            //type---std::vector<std::string>，用于存放切割之后的字符串
            //select_list---select_list：传入的字符串，可以为空
            //boost::is_any_of("")---设定切割符为
            //boost::token_compress_on/off---将连续多个分隔符当一个，默认没有打开，当用的时候一般是要打开的
            std::boost::split(*out,line,boost::is_any_of("sep"),boost::token_compress_on);            
        }
    };
}
