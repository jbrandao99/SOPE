#include <stdio.h>

int main(void)
{
    char nome[40];
    int numero;

    printf("Write a name: \n");
    fgets(nome, sizeof(nome), stdin);

    printf("Choose a number: \n");
    scanf("%d", &numero);

    for(unsigned int i = 0; i < numero; i++)
    {
    printf("Hello %s", nome);
    }
    
    return 0;
}