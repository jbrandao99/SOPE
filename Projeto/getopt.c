#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

char *filetype(char *opt)
{
  char *type;
  char *erro = "erro";
  type = opt;
  if (strcmp("md5", opt) == 0)
  {
    //IMPRESSAO DE MD5
    return type;
  }
  else if (strcmp("sha256 ", opt) == 0)
  {
    return type;
  }
  else if (strcmp("sha1 ", opt) == 0)
  {
    return type;
  }
  else if (strcmp("md5,sha1,sha256", opt) == 0)
  {
    return type;
  }
  else if (strcmp("sha1,sha256", opt) == 0)
  {
    return type;
  }
  else if (strcmp("md5,sha256", opt) == 0)
  {
    return type;
  }
  else if (strcmp("md5,sha1", opt) == 0)
  {
    return type;
  }
  else
  {
    return erro;
  }
}

int main(int argc, char *argv[])
{
int c;
char *ifile;
extern char *optarg;
if (argc == 2)
   {
     //forensic hello.txt
      printf("forensic hello.txt\n");

   }
while((c=getopt(argc,argv,"r:h:o:hov:")) != -1)
{
switch(c)
{
case 'r':
// forensic -r folder
// analisar todos os ficheiros do diretório indicado e de todos os seus subdiretórios.
printf("Analisando os ficheiros do diretorio: %s\n",optarg);
break;

case 'h':// forensic -h sha1,sha256 hello.txt
        //  forensic -h md5 -o output.txt -v hello.txt
// calcular uma ou mais “impressões digitais” dos ficheiros analisados. Pode pedir-se os algoritmos MD5,SHA1 ou SHA256; querendo-se mais do que um, separar os identificadores por vírgulas.
if(filetype(optarg)== "erro")
{
  exit(0);
}
printf("%s\n",filetype(optarg));
break;

case 'v':
// gerar ficheiro com os registos de execução, conforme explicado mais à frente. O nome do ficheiro é obtido da variável de ambiente LOGFILENAME.
printf("Gerando ficheiros com os registros de execucao\n");
//printf("%s\n",getenv("LOGFILENAME"));
break;

case 'o':
//armazenar os dados da análise no ficheiro indicado (e não na saída padrão). O ficheiro terá, naturalmente,o formato CSV (comma-separated values), devido à forma como a informação é apresentada.
printf("Data saved on file %s,\n",optarg); //output.txt)
printf("Execution records saved on file ...\n");
break;

case '?':
break;

default: //forensic hello.txt
break;
return 0;
}
}
}
