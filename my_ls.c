#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<limits.h>
#include<dirent.h>
#include<errno.h>
#include<pwd.h>
#include<grp.h>

#define PARAM_none 0    //无参数
#define PARAM_a    1    // -a: 显示所有文件，包括隐藏文件
#define PARAM_l    2    // -l: 显示文件属性
#define PARAM_R    4    // -R: 
#define MAXrow     80   // 一行显示的最多字符数

/* 错误处理函数，将错误信息和行数输出到终端 */
void err(const char *errors,int line)
{
	fprintf(stderr,"line:%d ",line);
	perror(errors);
	exit(1);
}

/* 获取文件属性 */
void attribute(struct stat sb)
{
	struct passwd *pwd;
	struct group *grp;
	char time[32],str_time[32];
	int i=4,j=0;
	/* 文件类型 */
	if(S_ISDIR(sb.st_mode))        //目录文件
	  printf("d");
	else if(S_ISREG(sb.st_mode))   //普通文件
	  printf("-");
	else if(S_ISCHR(sb.st_mode))   //字符文件
	  printf("c");
	else if(S_ISBLK(sb.st_mode))   //块文件
	  printf("b");
	else if(S_ISFIFO(sb.st_mode))  //管道文件
	  printf("p");
	else if(S_ISLNK(sb.st_mode))   //链接文件
	  printf("l");
	else if(S_ISSOCK(sb.st_mode))  //套接字文件
	  printf("s");
	/* 文件所有者权限 */
    if(sb.st_mode&S_IRUSR)        
	  printf("r");
	else
	  printf("-");
	if(sb.st_mode&S_IWUSR)
	  printf("w");
	else
	  printf("-");
	if(sb.st_mode&S_IXUSR)
	  printf("x");
	else
	  printf("-");

	/* 用户组权限 */
	if(sb.st_mode&S_IRGRP)
	  printf("r");
	else
	  printf("-");
	if(sb.st_mode&S_IWGRP)
	  printf("w");
	else
	  printf("-");
	if(sb.st_mode&S_IXGRP)
	  printf("x");
	else
	  printf("-");

	/* 其他用户权限 */
	if(sb.st_mode&S_IROTH)
	  printf("r");
	else
	  printf("-");
	if(sb.st_mode&S_IWOTH)
	  printf("w");
	else
	  printf("-");
	if(sb.st_mode&S_IXOTH)
	  printf("x");
	else
	  printf("-");
	printf("%4d ",sb.st_nlink);    //硬链接数
    
	pwd=getpwuid(sb.st_uid);
	grp=getgrgid(sb.st_gid);
	printf("%-8s",pwd->pw_name);   //用户
	printf("%-8s ",grp->gr_name);  //用户组
	printf("%-9ld",sb.st_size);    //节点大小
    
	/* 获取时间 */
    strcpy(str_time,ctime(&sb.st_mtime));
    for(i=4,j=0;i<19;i++,j++)
	  time[j]=str_time[i];
	time[j]='\0';
	printf("%s  ",time);
}

/* 没有-l选项时，严格对齐打印文件 */
void print(char filename[][NAME_MAX],int count,int param)
{    
	char str[NAME_MAX];
	int file_maxlen;
	int i=0,line,k,j,flag=0;
	k=i;
	/* 找到最长的文件名 */
	for(i=0;i<count;i++)
	{
		if(strlen(filename[k])<strlen(filename[i]))
		  k=i;
	}
	file_maxlen=strlen(filename[k]);
    line=MAXrow/(file_maxlen+2);
	if(line==0)
	{
		for(i=0;i<count;i++)
		{
			if(filename[i][0]=='.'&&param%4==PARAM_none)
			  continue;
			printf("%s\n",filename[i]);
			if(i==k)
			  printf("\n");
		}
	}
	else
	{
	for(i=0;i<count;i++)
	{
		if(filename[i][0]=='.'&&param%4==PARAM_none)
		  continue;
		printf("%s",filename[i]);
		flag++;
    	for(j=0;j<(file_maxlen-strlen(filename[i]));j++)
		  printf(" ");
		if(flag%line==0)
		  printf("\n");
		else
		  printf("  ");
	}
	printf("\n");
	}
}

/* 根据命令行参数和完整路径名显示各文件 
 * param                     命令行参数
 * pathname                  完整路径名
 */
void display(int param,char pathname[][PATH_MAX],int count)
{
	void get_dir(int param,char *path);
	int i,j,k;
	char name[256][NAME_MAX];
	struct stat sb;
	/* 从完整路径中解析出文件名 */
	for(k=0;k<count;k++)
	{
		for(i=0,j=0;i<strlen(pathname[k]);i++)
		{
			if(pathname[k][i]=='/')
			{
				j=0;
				continue;
			}
			name[k][j++]=pathname[k][i];
		}
		name[k][j]='\0';
	}

	/* 用lstat 方便解析链接文件 */
	switch(param%4)
	{
		case PARAM_none:               //无参数
			print(name,count,param);
			break;
		case PARAM_a:                  //有-a,无-l
            print(name,count,param);
			break;
		case PARAM_l:                  //有-l,无-a
			for(i=0;i<count;i++)
			{
                if(name[i][0]=='.')
				  continue;
				if(lstat(pathname[i], &sb) == -1)
				  err("stat",__LINE__);
				attribute(sb);
				printf("  %s\n",name[i]);
			}
			break;
		case PARAM_a+PARAM_l:          // -a -l
		    for(i=0;i<count;i++)
            {
				if(lstat(pathname[i], &sb) == -1)
			       err("stat",__LINE__);
			    attribute(sb);
			    printf("  %s\n",name[i]);
            }
	       break;
		default:
		   break;
	}
	if(param&PARAM_R)          //参数中有-R
	{
		for(i=0;i<count;i++)
	    {
            if(lstat(pathname[i], &sb) == -1)
                err("stat",__LINE__);
		    if(S_ISDIR(sb.st_mode))    //该子文件是目录
		    {
				if(((param&PARAM_a)==0)&&(name[i][0]=='.'))
					continue;
				if((param&PARAM_a)&&((strcmp(name[i],".")==0||strcmp(name[i],"..")==0)))
				    continue;
                strcat(pathname[i],"/");
				printf("\n");
			    get_dir(param,pathname[i]);
		    } 
		}
	}
}

/* 读取目录下的文件 */
void get_dir(int param,char *path)
{
	DIR  *dir;
	struct dirent *ptr;
	int count=0,i=0,j;
	char pathname[256][PATH_MAX],str[PATH_MAX];

	/* 获取目录下所有的文件名 */
	dir=opendir(path);
	if(dir==NULL)
	  err("stat",__LINE__);
	while((ptr=readdir(dir))!=NULL)
	{
		strcpy(pathname[i],path);
		strcat(pathname[i],ptr->d_name);
		pathname[i++][strlen(path)+strlen(ptr->d_name)]='\0';
		count++;
	}
	/* 按文件名排序 */
	for(i=0;i<count-1;i++)
	{
		for(j=0;j<count-1-i;j++)
		{
			if(strcmp(pathname[j],pathname[j+1])>0)
			{
				strcpy(str,pathname[j]);
				strcpy(pathname[j],pathname[j+1]);
				strcpy(pathname[j+1],str);
			}
		}
	}
	printf("%s:\n",path);
    display(param,pathname,count);
	printf("\n");
	closedir(dir);
}
int main(int argc,char *argv[])
{
	int i,j,k,num;
	char path[PATH_MAX],pathname[1][PATH_MAX];
	char c_param[32];                //保存所有参数
	int param=PARAM_none;            //用数值代表参数
	struct stat sb;
	/* 获取所有参数 -a,-l等 */
	j=0;
	num=0;
	for(i=1;i<argc;i++)
	{
		if(argv[i][0]=='-')
		{
		  for(k=1;k<strlen(argv[i]);k++,j++)
			c_param[j]=argv[i][k];
		  num++;                      //保存“-”的个数
		}
	}
	c_param[j]='\0';
    /* 检测参数选项是否合法 */
	for(i=0;i<j;i++)
	{
		if(c_param[i]=='a')
		{
			param=param|PARAM_a;
		}
		else if(c_param[i]=='l')
		{
			param=param|PARAM_l;
		}
		else if(c_param[i]=='R')
		{
			param=param|PARAM_R;
		}
		else
		{
			printf("my_ls: invalid option -%c\n",c_param[i]);
			exit(1);
		}
	}
	/* 如果没有输入文件名就显示当前目录 */
    if((num+1)==argc)
	{
		strcpy(path,"./");
		path[2]='\0';
		printf(".:\n");
		get_dir(param,path);
		return 0;
	}
	i=1;
	do
	{
		/* 如果不是文件和目录解析下一个命令行参数 */
		if(argv[i][0]=='-')
		{
			i++;
			continue;
		}
		else
		{
			strcpy(path,argv[i]);
			if(stat(path, &sb) == -1)  //文件或目录不存在
			  err("stat",__LINE__);
			if(S_ISDIR(sb.st_mode))    //是一个目录
			{
				if(path[strlen(argv[i])-1]!='/')//最后没有‘/’，补‘/’
				{
					path[strlen(argv[i])]='/';
					path[strlen(argv[i])+1]='\0';
				}
				else
				  path[strlen(argv[i])]='\0';
				get_dir(param,path);
				i++;
			}
			else                       //文件
			{
				strcpy(pathname[0],path);
				display(param,pathname,1);
				i++;
			}
		}
	}while(i<argc);
	return 0;
}
