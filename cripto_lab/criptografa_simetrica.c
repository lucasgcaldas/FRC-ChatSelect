#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/des.h>

#define BUFFER_SIZE 4096
#define HEADER_SIZE 54

void encryptFile(const char* inputFile, const char* outputFile, const char* key)
{
    FILE *fin = fopen(inputFile, "rb");
    FILE *fout = fopen(outputFile, "wb");

    unsigned char inputBuffer[BUFFER_SIZE];
    unsigned char outputBuffer[BUFFER_SIZE];

    DES_cblock desKey;
    DES_key_schedule keySchedule;

    memcpy(desKey, key, 8);
    DES_set_key((DES_cblock *)desKey, &keySchedule);

    // Ler e escrever o cabeçalho do arquivo BMP sem encriptar
    fread(outputBuffer, sizeof(unsigned char), HEADER_SIZE, fin);
    fwrite(outputBuffer, sizeof(unsigned char), HEADER_SIZE, fout);

    int bytesRead, bytesWritten;
    while ((bytesRead = fread(inputBuffer, sizeof(unsigned char), BUFFER_SIZE, fin)) > 0)
    {
        DES_ecb_encrypt((DES_cblock *)inputBuffer, (DES_cblock *)outputBuffer, &keySchedule, DES_ENCRYPT);
        bytesWritten = fwrite(inputBuffer, sizeof(unsigned char), bytesRead, fout);
        if (bytesWritten != bytesRead)
        {
            fprintf(stderr, "Erro ao escrever no arquivo de saída.\n");
            exit(1);
        }
    }

    fclose(fin);
    fclose(fout);

    printf("Encriptação concluída com sucesso.\n");
    printf("Chave de desencriptação: %s\n", key);
}

void decryptFile(const char *inputFile, const char *outputFile, const char *key)
{
    FILE *fin = fopen(inputFile, "rb");
    FILE *fout = fopen(outputFile, "wb");

    unsigned char inputBuffer[BUFFER_SIZE];
    unsigned char outputBuffer[BUFFER_SIZE];

    DES_cblock desKey;
    DES_key_schedule keySchedule;

    memcpy(desKey, key, 8);
    DES_set_key((DES_cblock *)desKey, &keySchedule);

    // Ler e escrever o cabeçalho do arquivo BMP sem desencriptar
    fread(outputBuffer, sizeof(unsigned char), HEADER_SIZE, fin);
    fwrite(outputBuffer, sizeof(unsigned char), HEADER_SIZE, fout);

    int bytesRead, bytesWritten;
    while ((bytesRead = fread(inputBuffer, sizeof(unsigned char), BUFFER_SIZE, fin)) > 0)
    {
        DES_ecb_encrypt((DES_cblock *)inputBuffer, (DES_cblock *)outputBuffer, &keySchedule, DES_DECRYPT);
        bytesWritten = fwrite(inputBuffer, sizeof(unsigned char), bytesRead, fout);
        if (bytesWritten != bytesRead)
        {
            fprintf(stderr, "Erro ao escrever no arquivo de saída.\n");
            exit(1);
        }
    }

    fclose(fin);
    fclose(fout);

    printf("Desencriptação concluída com sucesso.\n");
}

int main()
{
    const char *inputFile = "fractaljulia.bmp";
    const char *outputFile = "arquivo_encriptado.bmp";
    const char *outputFile2 = "arquivo_desencriptado.bmp";

    const char *key = "chave123";

    encryptFile(inputFile, outputFile, key);

    decryptFile(outputFile, outputFile2, key);

    return 0;
}
