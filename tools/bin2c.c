#include <stdio.h>


int bin2c(char *path)
{
	FILE *fp;		//FILE structer
	int  ch;		//read data
	int  idx=0;		//read index
	char var_name[256];	//variable name

	fp= fopen(path,"rb"); if(fp==NULL) {printf("file open error '%s'",path); exit(0);}
	ch = fgetc(fp); if( ch==EOF) {fclose(fp); return 0;}

	// get filename index
	int i;
	for(i=strlen(path)-1; i>=0 ; i--)
		{
		 if( path[i]=='\\') // filename 
			{
			i++;
			break;
			}
		}

	int j=0;
	for( ;i<strlen(path);i++)
		{
			{
			 if( path[i] !='.')
				{
		 		var_name[j]= path[i];
				j++;
				var_name[j]= 0;		// terminater
				}
			}
		} 

	printf("unsigned char compati_%s []= {\n",var_name);
	while(!feof(fp))
		{
		 printf("0x%02X,",ch);
		 idx++; if(idx>=16) {idx=0; printf("\n");}

		 ch = fgetc(fp); if( ch==EOF) break;
		}

	printf("};\n");
	fclose(fp);
	return(1);
}

int main(int argc,char *argv[])
{
	if( argc >1)
		{
		 bin2c(argv[1]);
		}
	else
		{
		 printf("usage: bin2c input-file >output-file \n");
		}
}
