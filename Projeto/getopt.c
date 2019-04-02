#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
int c;

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
printf("%s\n",optarg);
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
fprintf(stderr,"getopt");
break;


return 0;
}




}



}
