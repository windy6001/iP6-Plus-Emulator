// bdf2header.c
// 
// bdf ファイルから、データだけ抜き出して、ヘッダーに変換する

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//
//  CR を消す
void deleteCR( char *buff)
{
	int len= strlen(buff);
	for(int i=0;i<len;i++) {
		if(buff[i]=='\n') {
			buff[i] = 0;
		}
	}
}


// bdf file  ---> header file
int main(int argc, char *argv[])
{
	char path1[256] ,path2[256];
	char buff[256];
	int jiscode;

	if( argc >2) {
		strcpy( path1,argv[1]);
		strcpy( path2,argv[2]);
	}else {
		printf("usage: bdf2header bdf-file header-file\n");
		exit(0);
	}
	FILE *fp1= fopen(path1,"r"); if( fp1==NULL) {printf("file not found %s\n",path1); exit(0);}
	FILE *fp2= fopen(path2,"w"); if( fp2==NULL) {printf("file not found %s\n",path2); exit(0);}

	fprintf(fp2,"struct _fontdata {\n");
	fprintf(fp2,"	int code;\n");
	fprintf(fp2,"	unsigned short bitmap[14];\n");
	fprintf(fp2," } fontdata[] = {\n");

	int cnt=0;
	while( !feof(fp1)) {
		buff[0] = 0;
		fgets(buff, sizeof(buff) ,fp1);  // CODE
		deleteCR( buff);
		if( strncmp(buff,"STARTCHAR",9)==0) {
			fprintf(fp2,"0x%s,",buff+10);
			cnt++;

		} else if( strncmp(buff,"BITMAP",6)==0) {
			fprintf(fp2,"{");
			for(int j=0; j<14; j++) {
				fgets(buff,sizeof(buff),fp1);	// BITMAP DATA
				deleteCR( buff);
				fprintf(fp2,"0x%s,",buff);
				}
			fprintf(fp2,"},\n");
			}
		}
	fprintf(fp2,"};\n");
	fprintf(fp2,"int length =%d;\n",cnt);
	fclose(fp1);
	fclose(fp2);
	}


