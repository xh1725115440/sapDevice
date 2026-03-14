#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "aes_options.h" 
//#include <string>  //add
int main(){
	char *decry_string=NULL;
	char *encrypt_string=NULL;
	char s[2048]={0};
	char s1[2048]={0};
	//s[1024]="我是中国人";
	//String ss="我是中国人";
	//int len=strlen(ss);
	memset((char*)s,0,2048);
	memset((char*)s1,0,2048);
	strcpy(s,"我是一个中国人我是一个中国人我是一个dsfdsfdsfds654654654654中国人我是一个中国人fdfgfdgf63545654654yrtyyyrtrytr6546fgghgfhgf手机电视卡大厦的空间的就是第三方控件的是返回的是开发和第三方监督中国人我是gf63545654654国人中国人我gf635人我是一个dsfdsfdsfdgong美丽的钟爱的哈萨克记得发货防火等级考试辅导是否该u师父的话就是发货的设计风格带上飞机噶是否发货的设计风格的数据发给对方反对和顺丰单号设计风格地方都是地方海事局风格的和飞机喀什地方都是幅度和时间官方的说法决定是否给啊撒旦萨丢斯大赛业务你是编程模型下载");
	printf("%s\n",s); //
	encrypt(s,&decry_string);

	printf("%s\n",decry_string); //

	//char s1[(strlen(s1)/16+1)*16]={0};
	
	sprintf(s1,"%s",decry_string);
	int len=strlen(s1)+1;
	printf("%d\n",len);

	decrypt(s1,&encrypt_string,len);
	printf("%s\n",encrypt_string); //
	return 0;
}