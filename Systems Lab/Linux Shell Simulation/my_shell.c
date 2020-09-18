#include <unistd.h> 
#include<stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include<string.h>
#include<dirent.h>
#include<errno.h>
#include<sys/stat.h>
#include<time.h>
#define CWDSIZE 512
#define BASESIZE 256

char base_name[BASESIZE], *cwd, *cwd2s, *cod;
static char *line_read_c = (char *)NULL;

void ps(char *str)
{
	printf("%s\n", str);
}

//This functions assumes d to be octal and returns respective decimal number.
//It is used to use given number in mkdir mode.
int otd(int d)
{
	int i, o, j, arr[3];
	o=0;
	for(i=0; i < 3; i++)
	{
		arr[i] = (d%10);
		d/=10;
	}
	
	int n;

	for(i=0; i<3; i++)
	{
		n=arr[i];
		j=0;
		if(n>7)
			return -1;
		while(n)
		{
			//turn on required bit of o.
			if(n&1)
				o |= (1 << (i*3 + j));
			n = n>>1;
			j++;
		}
	}
	return o;
}

//If all is non zero it will print all directory.
void ls(char *direc, int all)
{
	DIR *dir;
	char *str;
	struct dirent *dirc;
	dir = opendir(direc);
	
	//List all the files in a directory.
	if(dir)
	{
		while((dirc = readdir(dir)))
		{
			str = dirc->d_name;
			if(str[0]=='.' && (!all))
				continue;
			ps(str);
		}
		closedir(dir);
	}
	else
		printf("Trouble opening directory.\n");
}

//Returns non zero number if str is a filename.
int isfile(char *str)
{
	struct stat strs;
	stat(str, &strs);
	return S_ISREG(strs.st_mode);
}

//This function is used to change current working directory.
//Home by default.
void cd(char *direc)
{
	int err;
	err = chdir(direc);
	switch(err){
		case EACCES:	printf("Permission Denied.\n");	break;
		case -1:
		case EFAULT:	printf("Incorrect Path.\n");		break;
		case ENOENT:	printf("File doesn't exist.\n");	break;
	}
	
}

//This function returns what ever input is given from keyboard.
char * rl_gets (char *pr)
{
  if (line_read_c!=NULL)
    {
      free(line_read_c);
      line_read_c = (char *)NULL;
    }

  line_read_c = readline (pr);

  if (line_read_c && *line_read_c)
    add_history (line_read_c);

  return line_read_c;
}

//This function takes 'current working directory' as input and
// modifies it to be shown in the terminal.
char * get_cwd(char *cwd)
{
	
	char *c;
	c = getcwd(cwd, sizeof(char)*CWDSIZE);
	
	if(c==NULL)
	{
		printf("Trouble getting directory!\n");
		exit(0);
	}
	
	c = (char *) malloc(sizeof(char)*CWDSIZE);
	
	//Replace /home/username with ~ to show in the terminal.
	if(cwd[1]=='h' && cwd[2]=='o' && cwd[3]=='m' && cwd[4]=='e' && cwd[5]=='/')
	{
		int i=6;
		while(cwd[i]!='\0')
		{
			if(cwd[i]=='/')
			{
				break;
			}
			i++;
		}
		strcpy(c, "~");
		strcat(c, cwd+i);	
	}
	else
		strcpy(c, cwd);
	
	return c;
}

//This function stores username and hostname in bn.
//It is needed only once.
void get_base_name(char *bn)
{
	char uname[256], hname[256];
	int uc, hc;
	uc = getlogin_r(uname, sizeof(uname));
	hc = gethostname(hname, sizeof(hname));

	if(uc==-1 || hc==-1 )
	{
		printf("Trouble getting environment names.\n");
		exit(0);
	}

	//Copies username to bn.
	strcpy(bn, uname);
	strcat(bn, "@");
	//Concatenates hostname to bn.
	strcat(bn, hname);
	strcat(bn, ":");

}

//It reads a file line by line and prints it on the screen.
//It might print the line number also, given the requirement.
void cat(char *fil, int n)
{
	if(!isfile(fil))
	{
		printf("%s is not a file.\n", fil);
		return;
	}

	FILE *fp;
	char s[2000];
	int spaces, tmp, coun=0;
	fp = fopen(fil, "r");

	//If cannot open the file.
	if(!fp)
	{
		printf("Error opening file.\n");
		return;
	}

	while(fgets(s, 2000, fp) != NULL)
	{
		//to print required number of spaces before the line number.
		//If asked.
		if(n)
		{
			coun++;
			tmp = coun;
			spaces = 6;
			while(tmp)
			{
				spaces--;
				tmp/=10;
			}
			while(spaces--)
				printf(" ");
			printf("%d\t", coun);
		}
		printf("%s", s);
	}

	fclose(fp);
}


//This function creates directory with fil name
//Its default mode is 755.
void mkd(char *fil, int mode)
{
	int err = mkdir(fil, mode);
	
	switch(err){
		case EACCES:	printf("Permission denied.\n");			break;
		case -1:
		case EEXIST:	printf("Filename already exists.\n");		break;
		case EROFS:		printf("Parent directory has read-only FS.\n");	break;
	}
}

//Copy files from f1 to f2.
//It copies in steps, 255 bytes at a time.
void cp(char *f1, char *f2)
{
	if(!isfile(f1))
	{
		printf("%s is not a file.\n", f1);
		return;
	}

	if(!isfile(f2))
	{
		printf("%s is not a file.\n", f2);
		return;
	}

	FILE *fp1, *fp2;
	fp1 = fopen(f1, "rb");
	fp2 = fopen(f2, "wb");

	//if cannot open either source or destination.
	if(fp1 == NULL || fp2==NULL)
	{
		printf("Error opening ");
		if(fp1==NULL)
			printf("Source file.\n");
		else
			printf("Destination file.\n");
		return;
	}

	unsigned char buff[255];
	int as;
	
	//reads file of max 255 bytes at a time.
	//then copies it to destination file.
	while( (as = fread(buff, 1, sizeof(buff), fp1)))
	{
		if(as<=0)
			break;
		fwrite(buff, 1, as, fp2);
	}

	fclose(fp1);
	fclose(fp2);
}

//This function compares one character to another. ignoring the case.
int cmpc(char a, char b)
{
	if(a>='a' && a<='z')
		a-=('a'-'A');

	if(b>='a' && b<='z')
		b-=('a'-'A');

	return a-b;
}

//This function is comparator to quicksort which takes two strings
//And returns -ve if x < y,
//		  0 if x==y,
//		  +v3 if x>y;
int cmp(const void *x,const void *y)
{
	int i, j, ret;
	char *a=*(char**)x;
	char *b=*(char**)y;
	i=j=0;

	//compare then unless one of them is different and neither is space or a tab.
	while(a[i]!='\0' && b[j]!='\0')
	{
		if((ret=cmpc(a[i], b[j]))==0)
		{
			i++;
			j++;
			continue;
		}
		
		//Ignoring spaces and tabs
		if(a[i]==' ' || a[i]=='\t')
		{
			i++;
			continue;
		}

		if(b[j]==' ' || b[j]=='\t')
		{
			j++;
			continue;
		}
		return ret;
	}

	while(a[i]==' ' || a[i]=='\t')
		i++;

	while(b[j]==' ' || b[j]=='\t')
		j++;

	if(a[i]=='\0' && b[j]=='\0')
		return 0;

	if(a[i]=='\0')
		return -1;

	return 1;
}

//This is the sorting function
//It sorts the file line by line using the comparator function.
//It may print in reverse order, given the requirement.
void sort(char *f, int r)
{
	if(!isfile(f))
	{
		printf("%s is not a file.\n", f);
		return;
	}

	FILE *fp = fopen(f, "r");
	int n=0;
	char *st[10000], *str=(char *)malloc(sizeof(char)*2048);

	if(fp==NULL)
		return;
	
	//Storing line in array of character pointers.
	while(fgets(str, 2048, fp))
	{
		if(n==10000)
			break;
		st[n++] = str;
		str = (char *)malloc(sizeof(char)*2048);
	}

	fclose(fp);
	
	//Sorting lines based on the comparator function.
	qsort((void *)st, n, sizeof(st[0]), cmp);

	//If we do not have to print in reverse order.
	if(!r)
	{
		for(int i=0; i<n; i++)
		{
			printf("%s", st[i]);
			free(st[i]);
		}
	}
	else
	{
		for(int i=n-1; i>=0; i--)
		{
			printf("%s", st[i]);
			free(st[i]);
		}
	}
}

//This command searches for a pattern in one or more files.
void grep(char *arg, int option)
{
	int ln=0, n=0;
	char *patt=strtok(arg, " ");
	char *fn[10];
	FILE *fp[10];
	
	for(int i=0; i<10; i++)
	{
		fp[i] = NULL;
		fn[i] = NULL;
	}

	//Open file pointers to all the file names.
	while((fn[n]=strtok(NULL, " ")))
	{
		fp[n] = fopen(fn[n], "r");
		n++;
		if(n==10)
			break;
	}
	
	//If there are no files.
	if(n==0)
	{
		printf("No filename entered.\n");
		return;
	}

	char str[2048];

	//For all the files one by one.
	for(int i=0; i<n; i++)
	{
		ln=0;

		//If file cannot be opened.
		if(fp[i]==NULL)
		{
			printf("%s: Error opening file.\n", fn[i]);
			continue;
		}

		while(fgets(str, 2048, fp[i]))
		{
			ln++;	
			if(strstr(str, patt)==NULL)
				continue;
			
			//If number of files is greater than one, then print file name with it.
			if(n>1)
				printf("%s:", fn[i]);
			//If we have to print line number also.
			if(option)
				printf("%d:", ln);
			
			//The line where patt is found.
			printf("%s", str);
		}
	}
}

//This function separates the commands its option and arguments.
//And then calls the respective functions if correct syntax.
void parse()
{
	int option=1, s=0;
	char *a=line_read_c;
	char *comm=strtok(a, " ");
	char *sec=strtok(NULL, " ");
	char *thir;
	char *arg = (char *) malloc(sizeof(char) * CWDSIZE);
	
	arg[0]='\0';
	
	if(sec==NULL)
		option=0;
	
	//If sec is not argument then, 	
	else if(sec[0] != '-')
	{
		option=0;
		strcat(arg, sec);
		s=1;
	}
	
	//Add remaining of string in arg.
	while((thir=strtok(NULL, " ")))
	{
		if(s)
		{
			strcat(arg, " ");
		}
		else
			s=1;
		strcat(arg, thir);
	}
	
	//if there is no option free
	if(option==0)
	{
		if(!sec)
			free(sec);

		sec=NULL;
	}
	
	if(comm==NULL)
		return;

	if(comm[0]=='l' && comm[1]=='s' && comm[2]=='\0')
	{
		//Check if there is an option it is in correct syntax.
		if(option)
		{
			if((sec[1]!='a' && sec[1]!='\0') || sec[2]!='\0')
			{
				printf("Wrong option.\n");
				return;
			}
		}

		//if there is no argument print current directory.
		ls(((arg[0] != '\0')?arg:"."), option);
	}
	
	else if(comm[0]=='c' && comm[1]=='d' && comm[2]=='\0')
	{
		//If just cd is entered, or entered with ~ then
		//go to home directory.
		if(strcmp(arg, "")==0 || strcmp(arg, "~")==0)
		{
			strcpy(arg, "/home/");
			char uname[256];
			getlogin_r(uname, sizeof(uname));
			strcat(arg, uname);
		}
		cd(arg);
	}
	
	else if(strcmp(comm, "cat")==0)
	{
		//Check if there is an option it is in correct syntax.
		if(option)
		{
			if((sec[1]!='n' && sec[1]!='\0') || sec[2]!='\0')
			{
				printf("Wrong option.\n");
				return;
			}
		}

		//if there is no argument.
		if(arg[0]=='\0')
		{
			printf("Enter file name.\n");
		}
		else
			cat(arg, option);
	}
	
	//If command is mkdir
	else if (strcmp(comm, "mkdir")==0)
	{
		int m=755;
		thir = NULL;

		//Check if there is an option it is in correct syntax.
		if(option)
		{
			if((sec[1]!='m' && sec[1]!='\0') || sec[2]!='\0')
			{
				printf("Wrong option.\n");
				return;
			}

			m = atoi(strtok(arg, " "));
			thir = strtok(NULL, "");
		}
		
		if(thir==NULL)
			thir = arg;

		//convert it to decimal from octal
		//for mode.
		m = otd(m);
		
		if(m==-1)
		{
			printf("Not an octal number.\n");
			return;
		}
		
		mkd(thir, m);
	}
	
	//if command is cp
	else if(strcmp(comm, "cp")==0)
	{
		//Check if there is an option it is in correct syntax.
		if(option)
		{
			if((sec[1]!='u' && sec[1]!='\0') || sec[2]!='\0')
			{
				printf("Wrong option.\n");
				return;
			}
		}

		sec=strtok(arg, " ");
		thir=strtok(NULL, "");
		
		//Checking if source file is newer or not.
		if(option)
		{
			struct tm *f1, *f2;
			struct stat at1, at2;
			stat(sec, &at1);
			stat(thir, &at2);
			f1 = gmtime(&(at1.st_mtime));
			f2 = gmtime(&(at2.st_mtime));
			if(f1->tm_year < f2->tm_year || f1->tm_mon < f2->tm_mon || f1->tm_mday < f2->tm_mday)
				return;
			if(f1->tm_hour < f2->tm_hour || f1->tm_min < f2->tm_min || f1->tm_sec <= f2->tm_sec)
				return;
		}

		cp(sec, thir);
	}
	
	//If command is exit
	else if(strcmp(comm, "exit")==0)
	{
		printf("\e[1;1H\e[2J");
		exit(0);
	}
	
	//If command is sort
	else if(strcmp(comm, "sort")==0)
	{
		//Check if there is an option it is in correct syntax.
		if(option)
		{
			if((sec[1]!='r' && sec[1]!='\0') || sec[2]!='\0')
			{
				printf("Wrong option.\n");
				return;
			}
		}

		sort(arg, option);
	}
	
	//If command is grep.
	else if(strcmp(comm, "grep")==0)
	{
		//Check if there is an option it is in correct syntax.
		if(option)
		{
			if((sec[1]!='n' && sec[1]!='\0') || sec[2]!='\0')
			{
				printf("Wrong option.\n");
				return;
			}
		}

		grep(arg, option);
	}
	
	else
		printf("Command not found.\n");
	
	if(!sec)
		free(sec);
	sec = NULL;
	
	if(!comm)
		free(comm);
	comm = NULL;
	
	if(arg)
		free(arg);
}
		

int main()
{
	cwd = (char *) malloc(sizeof(char)*CWDSIZE);
	char prompt[CWDSIZE*2];

	get_base_name(base_name);

	//cwd2s is used in prompt.
	cwd2s = NULL;

	//Clearing the screen.
	printf("\e[1;1H\e[2J"); 

	while(1)
	{
		if(cwd2s != NULL)
			free(cwd2s);
		cwd2s = get_cwd(cwd);
		
		//prompt is message displayed
		strcpy(prompt, base_name);
		strcat(prompt, cwd2s);
		strcat(prompt, "$ ");
		
		rl_gets(prompt);

		parse();
	}
}
