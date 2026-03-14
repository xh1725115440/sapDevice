#include "dataStore.h"
#include<iostream>
#define FILENAME  "data.txt"
#define MAXBYTE   512
//#define _GNU_SOURCE
int dataStore::creatDictory(const char* dictName) {
	//先检查目录是否存在
	if (mkdir(dictName, DIR_PERMS) == -1) {
		if (errno == EEXIST) {  //目录存在
			dictoryName = strcpy(dictoryName, dictName); //保存已经存在的路径
			return -1;
		}
		else {
			perror("mkdir:");
			return -2;//创建错误
		}
	}
	//dictoryName =  dictName; //创建成功保存目录路径
	dictoryName = strcpy(dictoryName, dictName); //保存已经存在的路径
	return 0;  //创建成功
}

int dataStore::getFileName() {
	DIR *dirp;
	struct dirent *dp;

	dirp = opendir(dictoryName); //获得当前目录下的目录流，其指向第一个文件
	if (dirp == NULL) {
		cout << "当前目录不存在" << endl;
		return -1;           //当前要查找的目录不存在
	}
	for (;;) {                //检测目录下的存在文件
		errno = 0;
		dp = readdir(dirp);
		if (dp == NULL) {  //读取到目录结尾，有错误发生返回
			break;
		}

		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
			continue;
		}
		else {      //返回找到的文件
			fileName = strcpy(fileName, dp->d_name);
			cout << "找到的文件名字：" << dp->d_name << endl;
			closedir(dirp);
			return 0;
		}
	}
	if (errno != 0) {
		strerror(errno);
		return -2; //有错误发生
	}
	closedir(dirp);
	if (strcmp(fileName, "") == 0) {  //此时未找到当前目录下有文件，创建文件
		string sdict = string(dictoryName) + "/" + string(FILENAME);
		string s = string(FILENAME);
		fileName = strcpy(fileName, s.c_str()); //设定文件名
		cout << "getFileName:" << fileName << endl;
		FILE *ff = fopen(sdict.c_str(), "w");
		if (ff == NULL) {
			perror("fopen:");
			return -2;
		}
		else {
			fclose(ff);
			return 0;
		}
	}
}
bool dataStore::writeToFile(const char* buff) {
	string s = string(dictoryName) + string("/") + string(fileName);
	const char * pathName = s.c_str();
	int fd = open(pathName, O_WRONLY | O_APPEND);
	if (fd < 0) {
		return -1;
	}
	char* buffer = (char *)malloc(strlen(buff) + 2); //为了填入换行符
	buffer = strcpy(buffer, buff);
	buffer[strlen(buff)] = '\n';            //加入换行符
	buffer[strlen(buff) + 1] = '\0';
	int ret = write(fd, buffer, strlen(buffer));
	if (ret == -1) {
		perror("write");
		close(fd);
		return false;
	}
	else {
		free(buffer);
		buffer = NULL;
		close(fd);
		return true;
	}
}
bool dataStore::readFile(char** buff) {
	string s = string(dictoryName) + "/" + string(fileName);
	const char * pathName = s.c_str();
	FILE *fs = fopen(pathName, "r");  //打开要读取的文本文件，以只读的方式打开
	if (fs == NULL) {
		perror("fopen:");
		return false;
	}
	size_t num = MAXBYTE;
	//*buff = (char*) malloc(num);   //先申请缓存空间
	//size_t num = MAXBYTE;
	*buff = NULL;
	size_t ret = myGetLine(buff, &num, fs);//一行一行地读取数据
	cout << "读到的字节数：" << ret << endl;
	fclose(fs);
	return true;
	/*以下代码功能段：为读取一行删除一行*/
	//off_t offset = lseek(fd, 0, SEEK_CUR);         //获取到当前fd文件偏移量
	//int fdtemp = open("tempfile", O_WRONLY | O_CREAT, DIR_PERMS);
	//if(fdtemp == -1){
	//	return 1   ;									 //
//	}
	//while(read(fd, uffer, 2048))
	//int ret write(fdtemp, )
}
size_t dataStore::myGetLine(char** buffer, size_t* num, FILE* sourstream) {
	*buffer = NULL;
	size_t count = 0;
	int buff;
	*buffer = (char *)malloc(*num);

	while ((buff = fgetc(sourstream)) != EOF) {
		if (buff == '\n') {         //意味着新的一行要开始了
			count += 1;
			return count;
		}
		count++;
		if (count >= *num) {       //重新申请内存
			char* tempbuff = (char *)malloc(strlen(*buffer) + 1);
			for (int i = 0; i < strlen(*buffer); i++) { //防止之前写入的数据丢失
				tempbuff[i] = (*buffer)[i];
			}
			tempbuff[strlen(*buffer)] = '\0';
			*buffer = (char *)realloc(*buffer, count * 2);
			for (int i = 0; i < strlen(tempbuff) + 1; i++) { //防止之前写入的数据丢失
				(*buffer)[i] = tempbuff[i];
			}
			free(tempbuff);
			tempbuff = NULL;
		}
		*(*buffer + count - 1) = buff;
		*(*buffer + count) = '\0';
	}
	return count;
}
dataStore::dataStore() {
	dictoryName = (char *)malloc(64); //设定文件目录最长为64个字节
	dictoryName = strcpy(dictoryName, "");
	fileName = (char *)malloc(64);	  //设定文件名最长为64个字节
	fileName = strcpy(fileName, "");
}