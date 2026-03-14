#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFFSIZE 512

void myGetLine(char** buffer, size_t* num, FILE* sourstream){
	//*buffer = NULL;
	*num = BUFFSIZE;
	static count = 0;
	int buff;
	*buffer = malloc(*num);
	while((buff = fgetc(sourstream)) != EOF){
		if(buff == '\n'){         //意味着新的一行要开始了
			count += 1;
			break;
		}
		count ++;
		if(count >= *num) {       //重新申请内存
			char* tempbuff = malloc(strlen(*buffer)+1);
			for(int i = 0 ;i < strlen(*buffer); i++){ //防止之前写入的数据丢失
				tempbuff[i] = buffer[i];
			}
			tempbuff[strlen(*buffer)+1] = '\0';
			realloc(*buffer, count * 2);
			for(int i = 0 ;i < strlen(*tempbuff)+1; i++){ //防止之前写入的数据丢失
				(*buffer)[i] = tempbuff[i];
			}
			free(tempbuff);
			tempbuff = NULL;
		} 
		*(*buffer+count-1) = buff;
		*(*buffer+count) = '\0';
	}
}