#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

struct Instruction
{
    /* data */
    string commond;
    string option;
    string parameter;
};

struct File
{
    /* data */
    string filename;    //文件名 test
    string path;        //路径 /etc/var/test
    int attr;           //文件属性，0-目录，1-文件
    int fstClus;        //文件开始的簇号
    int size;           //文件大小
    int depth;          //文件深度（用数组建立树形目录结构），根目录深度为0
};

struct RootEntry {
    /* data */
    char DIR_Name[8];            // 文件名8位
    char DIR_Attr[3];            // 扩展名3位
    char attribute;              // 文件属性
    char remain[10];             // 保留位
    char DIR_WrtTime[2];         // 最后一次写入时间
    char DIR_WrtDate[2];         // 最后一次写入日期
    unsigned short DIR_FstClus;  // 文件开始簇号
    unsigned int DIR_FileSize;   // 文件大小
};

string imageName = "a.img"; //镜像名称
File fileArray[4096];       //文件数组
int fileArraySize = 0;      //已存放文件数量
ifstream image;             //读操作    

string strErrorImage="No Such Image ! ";
string strNoticeDefault="> ";
string strNoticeExit="Bye~";
string strErrorCommand="command not support : ";
string strErrorOption="only support option 'l',but catch : ";
string strErrorParameter="Too many parameters !";
string strErrorFindPath="dir or file not exist : ";
string strErrorNotDir="it is not a dir : ";
string strErrorCat="it is not a txt file : ";
string strErrorCatOption="cat not support option : ";
string strBorkenCluster="There is a borken cluster !";

// extern "C" {void my_print(const char *str, int color);}

Instruction parseInstruction(string);
void split(string,vector<string> &,string);
void execLs(Instruction);
void fileDFS(int, bool);
void subNums(vector<int> &,int);
void execCat(Instruction);
void catFile(int);
int getNextCluster(int);
string getFileName(RootEntry &);
bool readImage(string);
void readDir(File &);
void print(string s){
    cout << s << endl;
}
void printRed(string s){
    cout << s << endl;
}
//main方法
int main(){
    //读取.img文件
	if (!readImage(imageName)) {
        print(strErrorImage+"\n");
        return 0;
    }
    //循环获取输入命令
    string inputstr = "ls";
    while (true) {
        print(strNoticeDefault);
        // getline(cin, inputstr);
        //读取到exit命令，退出程序
        if (inputstr == "exit") {  
            print(strNoticeExit+"\n");
            break;
        }
        //解析输入的命令
        Instruction instruction= parseInstruction(inputstr);
        //参数错误，不执行
        if (instruction.commond == "parameterError"){  
            print(strErrorParameter+"\n");
            continue;
        }
        //只支持两种命令：cat、ls，其他命令则报错
        if(instruction.commond=="cat"){
            execCat(instruction);
        }
        else if(instruction.commond=="ls"){
            execLs(instruction);
        }
        else{
            print(strErrorCommand+instruction.commond+"\n");
        }
    }
    //结束访问。关闭镜像，退出程序
    image.close();
    return 0;
}

//解析输入的命令字符串，返回指令struct
Instruction parseInstruction(string str){
    Instruction res;
    vector<string> v;
    split(str,v," ");
    res.commond=v[0];
    for(int i=1;i<v.size();i++){
        if(v[i][0]=='-'){  // 处理option（可以出现在指令的任意位置）
            res.option += v[i].substr(1);
        }
        else{  //处理parameter参数，但是只能有一个路径
            if(res.parameter==""){
                if(v[i][0]=='/')res.parameter=v[i];
                else res.parameter='/'+v[i];
            }
            else{
                //多条路径，commond改为error并直接返回
                res.commond="parameterError";
                return res;
            }
        }
    }
    if(res.parameter=="")res.parameter="/";  //默认路径为‘/’
    return res;
}

//字符串切分函数
void split(string str,vector<string> &v,string spacer)
{
    int pos1,pos2;
    int len=spacer.length();     //记录分隔符的长度
    pos1=0;
    pos2=str.find(spacer);
    while( pos2 != string::npos )
    {
        cout << str.substr(pos1,pos2-pos1) << endl;
        v.push_back( str.substr(pos1,pos2-pos1) );
        pos1 = pos2 + len;
        pos2 = str.find(spacer,pos1);    // 从str的pos1位置开始搜寻spacer
    }
    if(pos1 != str.length()) //分割最后一个部分
       v.push_back(str.substr(pos1));
}

//执行ls指令
void execLs(Instruction instruction){
    //检查option选项是否正确
    for(int i=0;i<instruction.option.length();i++){
        if(instruction.option[i]!='l'){
            print(strErrorOption+instruction.option[i]+"\n");
            return;
        }
    }
    //定位parameter所在路径
    int pos=-1;
    int parameter_dapth = -1;
    vector<string> v;
    int count = 0;
    split(instruction.parameter, v, "/");
    string sub_parameter = "";
    for(int i=0;i<v.size();i++){
        cout << v[i] << endl;
        if(v[i] == ".."){
            (parameter_dapth > 0) ? parameter_dapth-- : 0;
            count++;
        } 
        if(v[i][0] != '.') parameter_dapth++;
    }
    if(parameter_dapth == 0){
        pos = 0;
    }
    else{
        for(int i=v.size()-1;i>=1;i--){
            if(v[i] != ".." && v[i] != "."){
                sub_parameter = v[i];
                count--;
                if(count == -1)
                    break;
            }
        }
        for(int i=0;i<fileArraySize;i++){
            cout << fileArray[i].path << endl;
            int flag1 = fileArray[i].path.find(sub_parameter);
            bool flag2 = (fileArray[i].depth==parameter_dapth);
            if((flag1!=-1) && flag2){
                pos=i;
                break;
            }
            else pos = -1;
        }
    }
    if(pos==-1){  //路径不存在
        print(strErrorFindPath+instruction.parameter+"\n");
        return;
    }
    File file=fileArray[pos];
    //如果是文件夹则深度优先遍历DFS，如果是普通文件则报错：参数非目录
    if(file.attr==0){
        fileDFS(pos,instruction.option.length()!=0); //第二个参数表示是否有option，即指令是否有-l
    }
    else{
        print(strErrorNotDir+instruction.parameter+"\n");
    }
}

//文件深度优先搜索，根据haveOption参数选择输出简略信息/详细信息
void fileDFS(int pos,bool haveOption){
    File &file=fileArray[pos];
    string temp="";
    //打印当前路径信息
    temp+=file.path;
    if(file.depth>0)temp+="/";
    if(haveOption){
        vector<int> v;
        subNums(v,pos);
        temp+="  "+to_string(v[0])+" "+to_string(v[1]);
    }
    temp+=":\n";
    print(temp);
    //非根目录，打印 “.” 和 “..”
    if (pos != 0)
    {
        temp="";
        if(haveOption){
            temp=".\n..\n"; 
        }
        else{
            temp=".  ..  ";
        }
        printRed(temp);
    }
    //打印子文件和子目录
    bool needMoreEnter=false;
    for(int i=pos+1;i<fileArraySize;i++){
        temp="";
        File &current=fileArray[i];
        if(current.depth==file.depth)break;
        if(current.depth==file.depth+1){
            if(current.attr==0){  //current为目录
                printRed(current.filename+"  ");
                if(haveOption){
                    vector<int> v;
                    subNums(v,i);
                    temp="  "+to_string(v[0])+" "+to_string(v[1])+"\n";
                    print(temp);
                }
            }
            else{  //current为文件
                temp=current.filename+"  ";
                if(haveOption)temp+=to_string(current.size)+"\n";
                print(temp);
            }
        }
    }
    if(!haveOption)print("\n");
    //递归打印子目录的直接子文件和直接子目录
    for(int i=pos+1;i<fileArraySize;i++){
        File &current=fileArray[i];
        if(current.depth==file.depth)break;
        if(current.depth==file.depth+1 && current.attr==0){
            fileDFS(i,haveOption);
        }
    }
}

//遍历统计当前路径，直接子目录和直接子文件的数量
void subNums(vector<int> &v,int pos){
    int dirNum = 0;
    int fileNum = 0;
    File &dir = fileArray[pos];
    for (int i = pos + 1; i < fileArraySize; i++) {
        File &current = fileArray[i];
        if (current.depth == dir.depth)break;
        //直接子目录数量记录
        if (current.depth == dir.depth + 1 && current.attr == 0) {
            dirNum++;
        }
        //直接子文件数量记录
        if (current.depth == dir.depth + 1 && current.attr == 1) {
            fileNum++;
        }
    }
    v.push_back(dirNum);
    v.push_back(fileNum);
}


//执行cat指令
void execCat(Instruction instruction){
    if(instruction.option.length()>0){
        print(strErrorCatOption+instruction.option+"\n");
        return;
    }
    //定位parameter所在路径
    int pos=-1;
    for(int i=0;i<fileArraySize;i++){
        if(fileArray[i].path==instruction.parameter){
            pos=i;
            break;
        }
    }
    if(pos==-1){  //路径不存在
        print(strErrorFindPath+instruction.parameter+"\n");
        return;
    }
    File file=fileArray[pos];
    if(file.attr==0){  //目录
        print(strErrorCat+file.path+"\n");
    }
    else{  //文件
        catFile(pos);
    }
}

//cat指令读取文件
void catFile(int pos){
    File &file=fileArray[pos];
    char contain[32768];  // 512*8*8
    int cluster=file.fstClus;
    int bytes=0;
    while(cluster!=-1){
        long start=(cluster + 31) * 512;  //文件起始地址
        image.seekg(start);
        image.read((char*)contain+bytes*512,512);  //每次读取一个扇区（512字节）
        cluster=getNextCluster(cluster);
        bytes++;
    }
    print(contain);
    print("\n");
}

//根据当前簇号获取下一簇号
int getNextCluster(int cluster) {
    if (cluster<2) return -1;  //簇从2号开始
    int num1 = 0, num2 = 0;
    char *p1 = (char *)&num1;
    char *p2 = (char *)&num2;
    int start = 512+cluster*1.5;  // 引导扇区512，每一个簇12bit=1.5byte
    int res = 0;
    //每两个簇，组成3个字节，判断是前一个还是后一个
    //小端存储，低地址存放低字节，例子：0xf0与0xff后一个f构成了第一个簇0xff0
    if (cluster % 2 == 0) {  //前一个  a1a2、b1b2=b2a1a2
        image.seekg(start);
        image.read(p1, 1);
        image.read(p2, 1);
        num2 &= 0x0f;   //位运算取低地址
        res = (num2 << 8) + num1; 
    } else {  // 后一个  a1a2、b1b2=b1b2a1
        image.seekg(start);
        image.read(p1, 1);
        image.read(p2, 1);
        num1 &= 0xf0;  //位运算取高地址
        res = (num1 >> 4) + (num2 << 4);
    }
    if (res >= 0xff8) return -1;  //值大于或等于0xFF8，表示当前簇已经是本文件的最后一个簇
    if (res == 0xff7) {  //值为0xFF7，表示它是一个坏簇
        print(strBorkenCluster+"\n");
        return -1;
    }
    return res;
}

//读取镜像文件
bool readImage(string imageName) {
    image.open(imageName);
    if (!image) {
        return false;
    }
    File root;
    root.path = "/";        //根目录路径
    root.filename = "/";    //根目录名
    root.attr = 0;          //根目录为目录属性
    root.size = 0;          //目录文件大小为0
    root.fstClus = -12;     //31-1-9-9=12，固定根目录区为12扇区
    root.depth = 0;         //根目录深度为0
    fileArray[fileArraySize++] = root;  //根目录作为fileArray中的第一项
    readDir(root);          //从根目录开始递归读取
    return true;
}

//读取根目录项，存入fileArray中
void readDir(File &dir) {
    RootEntry tmpEntry;
    RootEntry *tmpPtr = &tmpEntry;
    int start = (dir.fstClus + 31) * 512;  //起始地址
    for (int i = 0; i < 14; i++) {
        // 遍历第一个扇区里所有的目录项，一个目录项32byte，最多有16个目录项
        image.seekg(start + i * 32);
        image.read((char *)tmpPtr, 32);             // 读取32个byte存到tmpEntry里
        if (tmpEntry.DIR_Name[0] == '\0')break;     // 读到了空文件，读取结束
        if (tmpEntry.DIR_Name[0] == '.')continue;   //读取到.或..，读下一个目录项即可
        //读取文件名和路径
        File file;
        file.filename = getFileName(tmpEntry);
        if (dir.path == "/") {
            file.path = "/" + file.filename;
        } else {
            file.path = dir.path + "/" + file.filename;
        }
        //读取文件类型
        if (tmpEntry.attribute == 0x10) {  //dir：0x10
            file.attr = 0;
        } else {  // 0x20 file（linux）、0x00 file（win）
            file.attr = 1;
        }
        //读取文件起始簇，大小，深度
        file.fstClus = tmpEntry.DIR_FstClus;
        file.size = tmpEntry.DIR_FileSize;
        file.depth = dir.depth + 1;
        //将文件加入fileArray中
        fileArray[fileArraySize++] = file;
        if (file.attr == 0) {
            // 如果是文件夹就递归读取目录
            readDir(file);
        }
    }
}

//处理文件名
string getFileName(RootEntry &tmpEntry) {
    string fileName = tmpEntry.DIR_Name;
    string attr = tmpEntry.DIR_Attr;
    for(int i=0;i<fileName.length();i++){
        if(fileName[i]==' '){
            fileName=fileName.substr(0,i);
            break;
        }
    }
    if (tmpEntry.attribute == 0x10) {  //dir：0x10，目录名8字节
        return fileName;
    } 
    else{  // 0x20 file（linux）、0x00 file（win），文件名8字节+3字节拓展名
        return fileName+"."+attr;
    }
}

//利用my_print.asm打印
// void print(string s) { my_print(s.c_str(), 0); }
// void printRed(string s) { my_print(s.c_str(), 1); }