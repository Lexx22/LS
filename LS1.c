/*
 * Без имени.c
 * 
 * Copyright 2014 Lex <lex22@ubuntu>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


char* GetFullPatch(char* FullName, char* DirectoryName, struct dirent* Dirent) {
	strcpy(FullName, DirectoryName);
	strcat(FullName, Dirent->d_name);		
	strcat(FullName,"\0");
	return FullName;
}

void WriteRight(struct stat inf, int FlagOfDirectory){ // Define access rights
	char right[11] = "----------\0";

	if (FlagOfDirectory) right[0] = 'd';

	if ( inf.st_mode & S_IRUSR ) right[1] = 'r';    
	if ( inf.st_mode & S_IWUSR ) right[2] = 'w';
	if ( inf.st_mode & S_IXUSR ) right[3] = 'x';

	if ( inf.st_mode & S_IRGRP ) right[4] = 'r';    
	if ( inf.st_mode & S_IWGRP ) right[5] = 'w';
	if ( inf.st_mode & S_IXGRP ) right[6] = 'x';

	if ( inf.st_mode & S_IROTH ) right[7] = 'r';    
	if ( inf.st_mode & S_IWOTH ) right[8] = 'w';
	if ( inf.st_mode & S_IXOTH ) right[9] = 'x';

	printf("%s ", right);
}


void WriteInformation(char* NameOfDirectory, struct dirent* Dirent, int FlagOfDirectory){
	struct stat inf;
	char* FullPath;
	int size = (strlen(NameOfDirectory) + strlen(Dirent->d_name) + 1) * sizeof(char);
	FullPath = malloc(size);
	GetFullPatch(FullPath, NameOfDirectory, Dirent);
	stat(FullPath, &inf);
	free(FullPath);

	WriteRight(inf, FlagOfDirectory);	
	//Show count of hard-linkins on object 
	printf("%lu ", inf.st_nlink);
	//Show user	
	printf("%s ", getpwuid(inf.st_uid)->pw_name);
	//Show group
	printf("%s ", getgrgid(inf.st_uid)->gr_name);
	//Show size
	printf("%ld ", inf.st_size);
	//Show size of str
	/*char* data = (char*)asctime(localtime(&inf.st_mtime));
	data[strlen(data)-1] = '\0';
	printf("%s ", data);*/
}


int CountDirectory(char* DirectoryName){
  	DIR* directory = opendir(DirectoryName);
	if (directory == NULL){
		perror("Directory couldn't open");
		return -1;
	}
  	struct dirent* Dirent;	
  	Dirent = readdir(directory);
	int res = 0;
  	while (Dirent != NULL)
  	{
		struct stat inf;
		char* FullPath;
		int size = (strlen(DirectoryName) + strlen(Dirent->d_name) + 1) * sizeof(char);
		FullPath = malloc(size);
		GetFullPatch(FullPath, DirectoryName, Dirent);
		stat(FullPath, &inf);
		free(FullPath);
		Dirent = readdir(directory);

	}
	return res;
}


int ShowDirectory(char* DirectoryName, int flag[2]){
	printf("%s\n", DirectoryName);
	DIR* directory = opendir(DirectoryName);
	if (directory == NULL){
		perror("Directory couldn't open");
		return -1;
	}
  	struct dirent* Dirent;

	if (flag[0]) 
		printf("Total: %d\n",CountDirectory(DirectoryName) - 2);

  	Dirent = readdir(directory);
	while (Dirent != NULL){
		if (Dirent->d_name[0] != '.') {
			if (flag[0])
				WriteInformation(DirectoryName, Dirent, Dirent->d_type & DT_DIR);						
			printf ("	%s\n", Dirent->d_name);
		}
		Dirent = readdir(directory);
  	}
	printf("\n\n");
	if (errno!=0){
		perror("Directory ncouldn't read");
		return -3;			
	}
	int closd = closedir(directory);
	if (closd!=0){
		perror("Directory couldn't closes");
		return -2;
		};
	return 0;
}

int ls(char* DirectoryName, int flag[2]){
	ShowDirectory(DirectoryName, flag);
	if (flag[1]){	
		DIR* directory = opendir(DirectoryName);
		if (directory == NULL){
			perror("Directory not be open");
			return -1;
		}
  		struct dirent* Dirent;
  		Dirent = readdir(directory);
  		while (Dirent != NULL){
			if (Dirent->d_name[0] != '.') {
				if (Dirent->d_type & DT_DIR) {	
					int size = (strlen(DirectoryName) + strlen(Dirent->d_name) + 2) * sizeof(char);							
					char* NewDir = malloc(size); 
					strcpy(NewDir, DirectoryName);
					strcat(NewDir, Dirent->d_name);
					strcat(NewDir, "/");
					ls(NewDir, flag);
					free(NewDir); 
				}
			}
			Dirent = readdir(directory);
  		
		}
		if (errno != 0){
			perror("Directory couldn't read");
			return -3;			
		}
		int closd = closedir(directory);
		if (closd != 0){
			perror("Directory couldn't closes");
			return -2;
			}
	}
	return 0;
}

int main (int argc, char *argv[]){
	if (argc > 3) {
		write(1, "Too much arguments\n", 19);
	}
	else{
		int i = 1, flag[2] = {0, 0};		
		for (; i<argc; ++i){
			if (!strcmp(argv[i], "-r")) 
				flag[1] = 1;
			if (!strcmp(argv[i], "-l")) 
				flag[0] = 1;
		}
		ls("./", flag);			
		write(1, "\n", 1);	
	}	
	return 0;
}

