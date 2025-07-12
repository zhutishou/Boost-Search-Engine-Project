#include <iostream>
#include <string>
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
}
