#include "data8.h"
//#include "dataStore.h"
//#include "dataStore.h"
//#include "dataStore.h"
//#include "dataStore.h"
//#include "dataStore.h"
#define FILENAME  "data.txt"
#define MAXBYTE   512
//#define _GNU_SOURCE
int dataStore::creatDictory(const char* dictName) {
	//先检查目录是否存在
	if (mkdir(dictName, DIR_PERMS) == -1) {
		if (errno == EEXIST) {  //目录存在
			dictoryName = strcpy(dictoryName, dictName); //保存已经存在的路径
			return DICTEXIST;
		}
		else {
			perror("mkdir:");
			return CREATERR;//创建错误
		}
	}
	//dictoryName =  dictName; //创建成功保存目录路径
	dictoryName = strcpy(dictoryName, dictName); //保存已经创建成功的路径
	return CREATSUCEE;  //创建成功
}

int dataStore::creatDictory(const char * dictName, const char * confName)
{
	if (mkdir(dictName, DIR_PERMS) == -1) {
		if (errno == EEXIST) {  //目录存在
			dictoryName = strcpy(dictoryName, dictName); //保存已经存在的路径
			//return DICTEXIST;
		}
		else {
			perror("mkdir:");
			return CREATERR;//创建错误
		}
	}
	if (mkdir(confName, DIR_PERMS) == -1) {
		if (errno == EEXIST) {  //目录存在
			confDictName = strcpy(confDictName, confName); //保存已经存在的路径
			//return DICTEXIST;
		}
		else {
			perror("mkdir:");
			return CREATERR;//创建错误
		}
	}
	dictoryName = strcpy(dictoryName, dictName); //保存创建成功的路径
	confDictName = strcpy(confDictName, confName); //保存创建成功的路径
	return CREATSUCEE;
}

int dataStore::getFileName2() {    //一次性读完，并保存,同时会读取相应的配置文件
	int findFlag = 0;
	DIR *dirp;
	struct dirent *dp;
	dirp = opendir(dictoryName); //获得目录下的目录流，其指向第一个文件
	if (dirp == NULL) {
		cout << "当前目录不存在" << endl;
		return DICTNEXISET;           //当前要查找的目录不存在
	}
	for (;;) {                //检测目录下的存在文件
		errno = 0;
		dp = readdir(dirp);
		if (dp == NULL) {  //读取到目录结尾，有错误，发生返回
			break;
		}
		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
			continue;
		}
		else {      //按照顺序存储找到的文件名字
			for (int i = 0; i < MAXSIZE; i++) {
				if (mydata->enable[i] == false) {
					mydata->fileName[i] = (char*)malloc(sizeof(char)* (strlen(dp->d_name) + 1));  //分配足够大的空间，保证程序的健壮性，防止内存溢出
					mydata->fileName[i] = strcpy(mydata->fileName[i], dp->d_name);
					char *buff = (char*)malloc(64);
					int ret = getConfName(dp->d_name, &buff);
					if (ret == CONFNEXIST) {		//如果没有相对应的配置文件，创建
						string s = string(dp->d_name).substr(0, strlen(dp->d_name) - 4);
						if (ret = creatFile(s.c_str(), MODE_CONF) == EXCUFALSE) return EXCUFALSE;
						getConfName(dp->d_name, &buff);  //创建后重新获取配置文件名
					}
					const char* buffer = buff;
					mydata->confName[i] = (char*)malloc(sizeof(char)* (strlen(buff) + 1));
					mydata->confName[i] = strcpy(mydata->confName[i], buffer);//同时找相对应的配置文件进行存储
					mydata->enable[i] = true;
					free(buff);
					findFlag++;
					break;
				}
			}
		}
	}
	if (errno != 0) {
		perror("readdir:");
		closedir(dirp);
		return ERROCCURCE; //有错误发生
	}
	if (findFlag != 0) {
		closedir(dirp);
		return EXCUSUCEE;
	}
	else {   //文件夹为空，则开始创建文件，并初始化
		time_t t = time(NULL);       //返回系统的当前时间
		char *fileNameT = ctime(&t);   //生成临时日期字符串
		char* fileName = (char*)malloc(sizeof(char) * (strlen(fileNameT) + 5)); //分配空间
		size_t size = strlen(fileNameT);
		fileName = strncpy(fileName, fileNameT, size - 1);//去掉回车符号
		fileName[size - 1] = '\0'; //添加终止符号
		if (creatFile(fileName, MODE_CONF | MODE_TXT) == CREATSUCEE) {
			mydata->fileName[0] = (char*)malloc(sizeof(char)* (strlen(fileName) + 5));
			mydata->confName[0] = (char*)malloc(sizeof(char)* (strlen(fileName) + 6));

			mydata->fileName[0] = strcpy(mydata->fileName[0], strcat(fileName, ".txt"));
			mydata->confName[0] = strcpy(mydata->confName[0], strcat(fileName, ".conf"));
			mydata->enable[0] = true;
			free(fileName);
			closedir(dirp);
			sleep(1);
			return EXCUSUCEE;
		}
	}
	closedir(dirp);
	return FILENEXIST;
}
int dataStore::getFileName()
{
	int findFlag = 0;
	DIR *dirp;
	struct dirent *dp;
	dirp = opendir(dictoryName); //获得目录下的目录流，其指向第一个文件
	if (dirp == NULL) {
		cout << "当前目录不存在" << endl;
		return DICTNEXISET;           //当前要查找的目录不存在
	}
	for (;;) {                //检测目录下的存在文件
		errno = 0;
		dp = readdir(dirp);
		if (dp == NULL) {  //读取到目录结尾，有错误，发生返回
			break;
		}
		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
			continue;
		}
		else {      //按照顺序存储找到的文件名字
			for (int i = 0; i < MAXSIZE; i++) {
				if (mydata->enable[i] == false) {
					if (getOfConf(dp->d_name, ROW) < MAXROW || getOfConf(dp->d_name, ALREAD) < getOfConf(dp->d_name, ROW)) {  //未写完的文本，和未读完的文本，加入静态数组，等待后续操作
						mydata->fileName[i] = (char*)malloc(sizeof(char)* (strlen(dp->d_name) + 1));  //分配足够大的空间，保证程序的健壮性，防止内存溢出
						mydata->fileName[i] = strcpy(mydata->fileName[i], dp->d_name);
						char *buff = (char*)malloc(64);
						int ret = getConfName(dp->d_name, &buff);
						if (ret == CONFNEXIST) {		//如果没有相对应的配置文件，创建
							string s = string(dp->d_name).substr(0, strlen(dp->d_name) - 4);
							if (ret = creatFile(s.c_str(), MODE_CONF) == EXCUFALSE) return EXCUFALSE;
							getConfName(dp->d_name, &buff);  //创建后重新获取配置文件名
						}
						const char* buffer = buff;
						mydata->confName[i] = (char*)malloc(sizeof(char)* (strlen(buff) + 1));
						mydata->confName[i] = strcpy(mydata->confName[i], buffer);//同时找相对应的配置文件进行存储
						mydata->enable[i] = true;
						free(buff);
						findFlag++;
						break;
					}
				}
			}
		}
	}
	if (errno != 0) {
		perror("readdir:");
		closedir(dirp);
		return ERROCCURCE; //有错误发生
	}
	if (findFlag != 0) {
		closedir(dirp);
		return EXCUSUCEE;
	}
	else {   //若没有未读完和未写完的，则开始创建文件，并初始化
		time_t t = time(NULL);       //返回系统的当前时间
		char *fileNameT = ctime(&t);   //生成临时日期字符串
		cout << "1: " << fileNameT << endl;
		cout << "1: " << &fileNameT << endl;
		char* fileName = (char*)malloc(sizeof(char) * (strlen(fileNameT) + 5)); //分配空间
		size_t size = strlen(fileNameT);
		fileName = strncpy(fileName, fileNameT, size - 1);//去掉回车符号
		fileName[size - 1] = '\0'; //添加终止符号
		cout << "getFileName:" << fileName << endl;
		cout << strlen(fileName) << endl;
		if (creatFile(fileName, MODE_CONF | MODE_TXT) == CREATSUCEE) {
			mydata->fileName[0] = (char*)malloc(sizeof(char)* (strlen(fileName) + 5));
			mydata->confName[0] = (char*)malloc(sizeof(char)* (strlen(fileName) + 6));

			mydata->fileName[0] = strcpy(mydata->fileName[0], strcat(fileName, ".txt"));
			mydata->confName[0] = strcpy(mydata->confName[0], strcat(fileName, ".conf"));
			mydata->enable[0] = true;
			free(fileName);
			closedir(dirp);
			sleep(1);
			return EXCUSUCEE;
		}
	}
	closedir(dirp);
	return FILENEXIST;
}
int dataStore::creatFile(const char * FileName, MODE_CREAT mode)   //根据模式创建所对应的文件
{
	string sdict;
	sdict = "";
	int ret;
	int fd;
	char row[64];
	char pointer[64];
	char alread[64];

	switch (mode)
	{
	case MODE_CONF:
		sdict = sdict + string(confDictName) + "/" + string(FileName) + string(".conf");
		fd = open(sdict.c_str(), O_CREAT | O_TRUNC | O_APPEND | O_WRONLY, DIR_PERMS);
		if (fd == -1) perror("open:");
		sprintf(row, "ROW: 0\n");
		sprintf(pointer, "POINTER: 0\n");

		write(fd, row, strlen(row));
		write(fd, pointer, strlen(pointer));
		close(fd);
		return CREATSUCEE;
	case MODE_TXT:
		sdict = sdict + string(dictoryName) + "/" + string(FileName) + string(".txt");
		fd = open(sdict.c_str(), O_CREAT | O_TRUNC, DIR_PERMS);
		if (fd == -1) perror("open:");
		close(fd);
		return CREATSUCEE;
	case (MODE_CONF | MODE_TXT):
		sdict = sdict + string(dictoryName) + "/" + string(FileName) + string(".txt");
		fd = open(sdict.c_str(), O_CREAT | O_TRUNC, DIR_PERMS);
		if (fd == -1) perror("open:");
		close(fd);
		sdict = "";
		//////////////////////////////////////////////////////
		sdict =  sdict + string(confDictName) + "/" + string(FileName) + string(".conf");
		fd = open(sdict.c_str(), O_CREAT | O_TRUNC | O_APPEND | O_WRONLY, DIR_PERMS);
		if (fd == -1) perror("open:");
		sprintf(row, "ROW: 0\n");
		sprintf(pointer, "POINTER: 0\n");
		sprintf(alread, "ALREAD: 0\n");
		write(fd, row, strlen(row));
		write(fd, pointer, strlen(pointer));
		write(fd, alread, strlen(alread));
		close(fd);
		return CREATSUCEE;
	default:
		break;
	}
	return EXCUFALSE;
}
int dataStore::getConfName(const char* fileName, char** rebuff)   //*后续操作可以直接通过文件名定位到其配置文件*
{
	int findFlag = 0;
	const char* fName = fileName;
	DIR *dirp;
	struct dirent *dp;
	dirp = opendir(confDictName); //获得配置目录下的流，其指向第一个文件
	if (dirp == NULL) {
		cout << "当前目录不存在" << endl;
		closedir(dirp);
		return DICTNEXISET;           //当前要查找的目录不存在
	}
	for (;;) {                //检测目录下的存在文件
		errno = 0;
		dp = readdir(dirp);
		if (dp == NULL) {  //读取到目录结尾、有错误，发生返回
			break;
		}
		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
			continue;
		}
		else if (strncmp(fName, dp->d_name, strlen(fName) - 5) == 0) { //找到对应的配置文件  
			*rebuff = strcpy(*rebuff, dp->d_name);
			findFlag = 1;
			closedir(dirp);
			return EXCUSUCEE;
		}
	}
	if (errno != 0) {
		strerror(errno);
		closedir(dirp);
		return ERROCCURCE; //有错误发生
	}
	if (findFlag != 1) {  //未找到
		closedir(dirp);
		return CONFNEXIST;
	}
}
int dataStore::writeToFile(const char* buff) {   
	static int iNum = 1;
	string s = "";
	char* fileNameWrite;
	//写之前，先检查
	while(getOfConf(fileNameEn, ROW) >= MAXROW && iNum < MAXSIZE){
		if(mydata->fileName[iNum] != NULL){
			fileNameEn = mydata->fileName[iNum];
			iNum++;
		}else if(mydata->fileName[iNum] == NULL){  //遍历到末尾
			time_t tt = time(NULL);       //返回系统的当前时间
			char *fileNameTT = ctime(&tt);   //生成临时日期字符串
			fileNameWrite = (char*)malloc(sizeof(char) * (strlen(fileNameTT) + 5)); //分配空间
			size_t size = strlen(fileNameTT);
			fileNameWrite = strncpy(fileNameWrite, fileNameTT, size-1); 
			fileNameWrite[size-1] = '\0';
			sleep(1);
			creatFile(fileNameWrite, MODE_CONF | MODE_TXT);
			mydata->fileName[iNum] = (char *)malloc(sizeof(char) * (strlen(fileNameWrite) + 5));
			mydata->fileName[iNum] = strcpy(mydata->fileName[iNum], strcat(fileNameWrite, ".txt"));
 			fileNameEn = mydata->fileName[iNum];
			iNum++;
		}
	}
	s = s + string(dictoryName) + string("/") + string(fileNameEn);
	const char * pathName = s.c_str();
	int fd = open(pathName, O_WRONLY | O_APPEND);
	if (fd < 0) {
		return EXCUFALSE;
	}
	char* buffer = (char *)malloc(strlen(buff) + 2); //为了填入换行符
	buffer = strcpy(buffer, buff);
	buffer[strlen(buff)] = '\n';            //加入换行符
	buffer[strlen(buff) + 1] = '\0';
	int ret = write(fd, buffer, strlen(buffer));
	if (ret == -1) {
		perror("write");
		free(buffer);
		close(fd);
		return EXCUFALSE;
	}
	else {
		cout << "写入成功" << endl;
		free(buffer);
		buffer = NULL;
		cout<<s.substr(18)<<endl;
		if (upConfFile((s.substr(18)).c_str(), ROW, 1) == FILEISFULL) {
			if (mydata->fileName[iNum] != NULL && iNum < MAXSIZE ) {  //如果当前有可以执行写的文件
				fileNameEn = mydata->fileName[iNum];
				iNum++;
				cout << "切换到下一个文件写" << endl;
				return WRITEFINSH;
			}
			cout << "开始创建新文件" << endl;
			time_t t = time(NULL);       //返回系统的当前时间
			char *fileNameT = ctime(&t);   //生成临时日期字符串
			for (iNum; iNum < MAXSIZE; iNum++) {
				if (mydata->enable[iNum] == false) {
					fileNameWrite = (char*)malloc(sizeof(char) * (strlen(fileNameT) + 5)); //分配空间
					size_t size = strlen(fileNameT);
					fileNameWrite = strncpy(fileNameWrite, fileNameT, size - 1);
					fileNameWrite[size - 1] = '\0';
					sleep(1); //创建文件时，延迟一秒，防止文件重名造成覆盖	
					creatFile(fileNameWrite, MODE_CONF | MODE_TXT);
					mydata->fileName[iNum] = (char *)malloc(sizeof(char) * (strlen(fileNameWrite) + 5));
					mydata->fileName[iNum] = strcpy(mydata->fileName[iNum], strcat(fileNameWrite, ".txt"));
					fileNameEn = mydata->fileName[iNum]; //更新可写文件
					free(fileNameWrite);
					iNum++;
					break;
				}
			}
			cout << "文件创建成功" << endl;
		}
		close(fd);
		return EXCUSUCEE;
	}
}
int dataStore::readFile(char** buff) {
	static int iNumr = 1;
	static int ret;
	string s = "";
	s = string(dictoryName) + "/" + string(fileNameENr);
	const char * pathName = s.c_str();
	//先获取文件偏移指针,从获取的文件指针处开始读取
	int value = getOfConf(fileNameENr, POINTER);
	cout << "文件指针" << value << endl;
	FILE *fs = fopen(pathName, "r");  //打开要读取的文本文件，以只读的方式打开
	if (fs == NULL) {
		perror("fopen:");
		return EXCUFALSE;
	}
	fseek(fs, value, SEEK_SET);     
	size_t num = MAXBYTE;
	ret = myGetLine(buff, &num, fs);//一行一行地读取数据
	cout << "读到的字节数：" << ret << endl;
	while(ret == 0 && iNumr < MAXSIZE){
		if(mydata->fileName[iNumr] != NULL){
			fileNameENr = mydata->fileName[iNumr];//更新可读文件
			iNumr++;
			fclose(fs);
			//更新文件，再次读取
			s = string(dictoryName) + "/" + string(fileNameENr);
			pathName = s.c_str();
			fs = fopen(pathName, "r");
			value = getOfConf(fileNameENr, POINTER);
			fseek(fs, value, SEEK_SET);
			ret = myGetLine(buff, &num, fs);
		}else{         //读取完毕
			return FILEREADEND;
		}
	}
	//读取一行更新配置文件 1：先获取目前的文件指针 2：更新配置文件指针 3:更新已读行数
	int filePointerCur = ftello(fs);
	cout << "待更新的文件指针数值" << filePointerCur << endl;
	if ((upConfFile(fileNameENr, POINTER, filePointerCur) == EXCUSUCEE)) cout << "文件指针更新完毕" << endl;
	if ((upConfFile(fileNameENr, ALREAD, 1) == EXCUSUCEE)) cout << "已读行数更新完毕" << endl;
	fclose(fs);
	return EXCUSUCEE;
}
int dataStore::getEnableFile()
{
	int enFlag = 0;
	for (int i = 0; i < MAXSIZE; i++) {
		if (mydata->enable[i] == true) {
			fileNameEn = mydata->fileName[i];   //获取可执行操作的写相应文件名
			//confNameEn = mydata->confName[i];
			fileNameENr = mydata->fileName[i]; //获取可执行操作的读相应文件名
			enFlag = 1;
			break;
		}
	}
	if (enFlag == 1) return EXCUSUCEE;
	return EXCUFALSE;
}
size_t dataStore::myGetLine(char** buffer, size_t* num, FILE* sourstream) {
	size_t count = 0;
	int buff;

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
			free(*buffer);  //释放掉原始的内存，防止内存泄露
			(*buffer) = NULL;
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
int dataStore::upConfFile(const char* confName, const char* filed, int filedValue)
{
	int fd = 1;
	char* buff = NULL;
	char* newBuff = NULL;   //为了存放新修改的内容
	int size = 0;
	string fName = "";
	fName = fName + string(confDictName) + "/" + string(confName).substr(0, strlen(confName) - 4) + ".conf"; //获取对应配置文件的完整路径
	if ((fd = open(fName.c_str(), O_RDWR)) == -1) {
		perror("upConfFile open:");
	}
	else {
		char value[5];  
		off_t fileSize = lseek(fd, 0, SEEK_END); //获取文件的大小（比实际的大小多一个字节）
		if (fileSize == -1) perror("lseek:");
		buff = (char*)malloc(sizeof(char) * (fileSize + 5)); //动态分配空间防止溢出
		newBuff = (char*)malloc(sizeof(char) * (fileSize + 10)); //修改的内容比原先的可能
		lseek(fd, 0, SEEK_SET);
		int ret = read(fd, buff, fileSize);
		cout << "buff:" << buff << endl;
		buff[fileSize] = '$'; //自定义一个结束符
		cout << "buff:" << buff << endl;
		if (ret == -1) perror("read:");
		//为了传参，类型转换i
		char *temp = (char*)malloc(sizeof(char) * (strlen(filed) + 1));
		temp = strcpy(temp, filed);
		char *filedStart = strstr(buff, temp);    //以下功能代码为定位到要修改字段值的起始位置
		if (filedStart == NULL) perror("strstr:");
		filedStart += strlen(filed);
		while (*filedStart == ' ' || *filedStart == ':') filedStart++;  //跳过空格和：
		while ((buff + size) != filedStart) {
			*(newBuff + size) = *(buff + size);
			size++;
		}
		int count = 0;
		while (*filedStart >= '0' && *filedStart <= '9' && count < 4) {
			value[count] = *filedStart;
			filedStart++; //跳过原值
			count++;
		}
		value[count] = '\0';
		int num1 = atoi(value);
		char lastValue[5];
		if (filed == ROW || filed == ALREAD) {
			sprintf(lastValue, "%d", num1 + filedValue);
		}
		else
			sprintf(lastValue, "%d", filedValue);
		for (int i = 0; i < strlen(lastValue); i++) {
			*(newBuff + size) = *(lastValue + i);
			size++;
		}
		while ((*filedStart)) {
			if ((*filedStart) == '$') {
				*(newBuff + size) = '\0';
				break;
			}
			*(newBuff + size) = *filedStart;
			size++;
			filedStart++;
		}
		close(fd);
		if ((fd = open(fName.c_str(), O_WRONLY | O_TRUNC)) == -1) {
			perror("upConfFile open:");
			free(buff);
			free(newBuff);
			free(temp);;
			return EXCUFALSE;
		}
		cout << newBuff << endl;
		if ((ret = write(fd, newBuff, strlen(newBuff))) == -1) {
			perror("upConfFile write:");
			free(buff);
			free(newBuff);
			free(temp);
			return EXCUFALSE;
		}
		if ((num1 + filedValue) >= MAXROW) {
			free(buff);
			free(newBuff);
			free(temp);
			return FILEISFULL;  //单个文件超过存储的行数
		}
		close(fd);
		free(buff);
		free(newBuff);
		free(temp);
		return EXCUSUCEE;
	}
}
int dataStore::getOfConf(const char * confName, const char * filed)  //默认为数据文件名
{
	int fd;
	char* buff;
	string fName = string(confDictName) + "/" + string(confName).substr(0, strlen(confName) - 4) + ".conf"; //获取对应配置文件的完整路径
	if ((fd = open(fName.c_str(), O_RDONLY)) == -1) {
		perror("upConfFile open:");
	}
	else {
		char* value = (char*)malloc(sizeof(char) * 5);  //
		off_t fileSize = lseek(fd, 0, SEEK_END); //获取文件的大小（比实际的大小多一个字节）
		if (fileSize == -1) perror("lseek:");
		buff = (char*)malloc(sizeof(char) * fileSize); //动态分配空间防止溢出
		lseek(fd, 0, SEEK_SET);
		int ret = read(fd, buff, fileSize);
		if (ret == -1) perror("read:");
		//为了传参，类型转换i
		char *temp = (char*)malloc(sizeof(char) * (strlen(filed) + 1));
		temp = strcpy(temp, filed);
		char *filedStart = strstr(buff, temp);    //以下功能代码为定位到要修改字段值的起始位置
		if (filedStart == NULL) perror("strstr:");
		filedStart += strlen(filed);
		while (*filedStart == ' ' || *filedStart == ':') filedStart++;  //跳过空格和：
		char *tempValue = value;
		while (*filedStart >= '0' && *filedStart <= '9') {
			*value = *filedStart;
			filedStart++; //跳过原值
			value++;
		}
		*value = '\0';
		int num1 = atoi(tempValue);
		free(tempValue);
		free(buff);
		free(temp);
		return num1;
	}
	return 0;
}
dataStore::dataStore() {
	dictoryName = (char *)malloc(64); //设定文件目录最长为64个字节
	confDictName = (char *)malloc(64);
	//fileNameEn = (char *)malloc(64);
	fileNameEn = NULL;
	confNameEn = (char *)malloc(64);
	newFileName = NULL;
	newConfName = NULL;
	fileNameENr = NULL;
	mydata = new myFileData();        //初始化自定义结构体
	for (int i = 0; i < MAXSIZE; i++) {
		mydata->confName[i] = NULL;
		mydata->fileName[i] = NULL;
		mydata->enable[i] = false;
	}
	//首先检查两个文件夹是否存在,不存在则创建
	if (creatDictory(DICTORYNAME, CONFDICTNAME) == CREATSUCEE) cout << "文件夹检查成功" << endl;
	//开始读取数据文件及其配置文件的名字
	if (getFileName() == EXCUSUCEE) cout << "数据文件及其配置文件名字读取成功" << endl;
	//获取一个可执行操作的数据文件及其配置文件
	if (getEnableFile() == EXCUSUCEE)
		cout << "初始化完毕" << endl;
	else cout << "请手动添加可操作的数据文件及其配置文件" << endl;
	cout << "构造函数执行完毕" << endl;
}

int main() {
	dataStore *DA = new dataStore();
	const char* ss = "1111";
	int num = 20;
	while(num--){
		DA->writeToFile(ss);
	}
	char *buffer = (char *)malloc(MAXBYTE);
	while(DA->readFile(&buffer) != FILEREADEND){
		cout<<buffer<<endl;
	}
	return 0;
}




