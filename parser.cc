#include <iostream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp> //boost库头文件
#include <utility>
#include "util.hpp"

//不建议直接写using namespace std;

//读取位置与目的位置
const std::string src_path="data/input";//读取路径，将其中的html文件读取
const std::string output="data/raw_html/raw.txt";//将读取到的html内容合并形成一个新的txt文件

//定义一个结构体满足第二步解析形成的结构
typedef struct DocInfo
{
    std::string title;//定义文件标题
    std::string content;//定义文件内容
    std::string url;//定义文件url
}DocInfo_t;

//声明函数：
bool ReaderSuccess(const std::string& _src_path,std::vector<std::string> *_file_reader);
bool ParseHtml(const std::vector<std::string>& _file_reader,std::vector<DocInfo_t> *_result);
bool WriteHtml(const std::vector<DocInfo_t>& _result,const std::string& output);

int main()
{
    //1.将每个html文件名加路径读取到一个vector中，方便后续读取操作
    std::vector<std::string> file_reader;
    if(!ReaderSuccess(src_path,&file_reader))//判断读取是否成功
    {
        //error：
        std::cerr<<"File Reader name error"<<std::endl;
        return 1;//返回错误码为1
    }

    //2.将file_reader每个解析，生成：title+content+url的形式
    std::vector<DocInfo_t> result;
    if(!ParseHtml(file_reader,&result))//判断划分是否成功
    {
        //error：
        std::cerr<<"Parse Html error"<<std::endl;
        return 2;//返回错误码为2
    }

    //3.将解析的各文件写入到output中，并且以/3为分隔符
    if(!WriteHtml(result,output))//判断写入是否成功
    {
        //error：
        std::cerr<<"Write Html error"<<std::endl;
        return 3;//返回错误码为3
    }


    return 0;
}

//实现函数：

static bool ParseTitle(const std::string& result,std::string* title)//result,&doc.title
{
    //提取title本质就是找<title>......</title>中间的内容，利用[)区间找
    //找头
    std::size_t begin=result.find("<title>");//size_t find (const string& str, size_t pos = 0) const
    //检查
    if(begin==std::string::npos)
    {
        return false;
    }
    begin+=std::string("<title>").size();

    //找尾
    std::size_t end=result.find("</title>");
    //检查
    if(end==std::string::npos)
    {
        return false;
    }
    //检查
    if(begin > end)
        return false;
    *title=result.substr(begin,end-begin);
    return true;
}

static bool ParseContent(const std::string& result,std::string* content)
{
    //利用状态机来查找content
    enum status
    {
        LABEL_1,//<
        LABEL_2//>
    };
    enum status s=LABEL_1;
    for(char ch:result)
    {
        switch(s)
        {
            case LABEL_1://当为'>'时，记录为LABEL_2
                if(ch=='>')
                    s=LABEL_2;
                break;
            case LABEL_2://当为'<'时，记录为LABEL_1
                if(ch=='<')
                    s=LABEL_1;
                else
                {
                    //此时为>到<之间的内容是需要存取的
                    //注意：暂时我们先不存取'\n'，转换为' '存取
                    if(ch=='\n')
                        ch=' ';
                    content->push_back(ch);
                }
                break;
            default:
                break;
        }
    }

    return true;
}

static bool ParseUrl(const std::string& file_path,std::string* url)
{
    //通过url_head和url_tail拼接
    std::string url_head="https://www.boost.org/doc/libs/1_88_0/doc/html";//通过将boost库URL头部找到
    std::string url_tail=file_path.substr(src_path.size());//data/input这是我们自己的路径裁掉，后续部分和head拼接形成正确的路劲
    *url = url_head+url_tail;
    return true;
}

bool ReaderSuccess(const std::string& _src_path,std::vector<std::string> *_file_reader)
{
    //1.定义一个命名空间来使用path对象
    namespace bs=boost::filesystem;
    bs::path root_path(_src_path);//通过构造函数将_src_path路径构造给root_path

    //2.检查构造是否成功
    if(!bs::exists(root_path))//构造失败
    {
        std::cerr<<_src_path<<" is not exists"<<std::endl;
        return false;
    }

    //3.定义迭代器来进行递归，同时判断是否正常结束
    bs::recursive_directory_iterator end;
    for(bs::recursive_directory_iterator iter(root_path);iter!=end;iter++)//遍历
    {
        //判断path是否为常规路径
        if(!bs::is_regular_file(*iter))
        {
            continue;
        }
        //判断是否是以".html"结尾文件
        if(iter->path().extension()!=".html")
        {
            continue;
        }
        //暂时先打印观察：
        //std::cout<<"debug:"<<iter->path().string()<<std::endl;
        //当前路径文件合法，放入_file_reader中
        _file_reader->push_back(iter->path().string());
    }

    return true;
}

//only for debug:
static void ShowDoc(const DocInfo_t& doc)
{
    std::cout<<"title: "<<doc.title<<std::endl;
    std::cout<<"content: "<<doc.content<<std::endl;
    std::cout<<"url: "<<doc.url<<std::endl;
}

bool ParseHtml(const std::vector<std::string>& _file_reader,std::vector<DocInfo_t>* _result)
{
    for(const std::string& file:_file_reader)
    {
        //1.利用util中ReadFile函数读取文件
        std::string result;
        if(!ns_util::FileUtil::ReadFile(file,&result))//读取，并判断是否读取正确
        {
            continue;
        }

        DocInfo_t doc;
        //2.解析指定文件，获取其title
        if(!ParseTitle(result,&doc.title))//判断是否解析错误
        {
            continue;
        }

        //3.解析指定文件，获取其content---简称去标签
        if(!ParseContent(result,&doc.content))//判断是否解析错误
        {
            continue;
        }

        //4.解析指定文件，获取其url
        if(!ParseUrl(file,&doc.url))//判断是否解析错误
        {
            continue;
        }

        //临时展示，方便调试：
        //ShowDoc(doc);

        //将上述工作完成的doc写入类型为DocInfo_t的vector中
        _result->push_back(std::move(doc));
        //注意点:这里用move可以减少不必要的构造，减少资源浪费，同时在这里由于doc可能过大，利用move实现资源转移，可以更加高效
        //move 头文件 <utility>
    
    }
    return true;
}

bool WriteHtml(const std::vector<DocInfo_t>& _result,const std::string& output)
{
#define SEP '\3'
    //通过文件写入到output
    std::ofstream out(output,std::ios::out|std::ios::binary);
    //检查是否打开成功
    if(!out.is_open())
    {
        std::cerr<<"open "<<output<<" is failed !"<<std::endl;
        return false;
    }
    //读取：方式为title\3content\3url \n title\3content\3url
    for(auto& s:_result)
    {
        std::string out_string;
        out_string=s.title;
        out_string+=SEP;
        out_string+=s.content;
        out_string+=SEP;
        out_string+=s.url;
        out_string+='\n';

        //写入：
        out.write(out_string.c_str(),out_string.size());
    }

    //关闭文件
    out.close();
    return true;
}
