#include "misc.h"

void strrncpy(char *str,char *str2,long n)
{
//str-reverse-n-cpy (strips n chars from the beginning and takes care of tabs)

	if(strspn(str2,"\t")!=0)
	{
		str[0]=0;
		while(*str2)
		{
			if(*str2=='\t')strcat(str,"        ");
			else strncat(str,str2,1);
			str2++;
		}
		strcpy(str2,str);
	}
	strcpy(str,&str2[n-1]);
}
char *strhtml(char *str)
{
//<pre>
//mitten in zeile ersetzen
//frames support
	int ignore=0,x=0;
	char buf[4096],buf2[4096],buf3[4096];

	stplcr(str);
	strcpy(buf2,str);
	
	if(!access(buf2,F_OK))
	{
		
//		if(stricmp(&buf2[strcspn(buf2,".")],"jpg")||stricmp(&buf2[strcspn(buf2,".")],"gif"))			sprintf(str,"<img src=\"%s\">",buf2);
//		else 
sprintf(str,"<a href=\"%s\">%s</a>",buf2,buf2);
	}
	else if(strchr(buf2,32)!=NULL)
	{
		strcpy(buf3,buf2);
		buf3[strcspn(buf3," ")]=0;
		if(!access(buf3,F_OK))
		{
			sprintf(str,"<a href=\"%s\">%s</a>%s",buf3,buf3,&buf2[strcspn(buf2," ")]);
		}
	}

	x=strlen(str);
	buf2[0]=0;
	while(*str)
	{
		buf[0]=0;

		if(*str=='>')ignore=0;
		if(!ignore)
		{
			if(*str=='<')ignore=1;
			if(*str==32)strcpy(buf,"&nbsp;");//nicht bei nur einem space
			if(*str=='\t')
			{
				while(!((x-strlen(str))%8))
				strcat(buf,"&nbsp;");
//				strcpy(buf,"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
			}
		}
		
		if(!buf[0])sprintf(buf,"%c",*str);

		strcat(buf2,buf);
		str++;
	}
	strcat(buf2,"<br>\n");

	strcpy(str,buf2);
	return(str);
}
