#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/des.h>

#define BUFFER_SIZE 4096
#define HEADER_SIZE 54

void encryptFile(const char *inputFile, const char *encryptedFile, const char *key)
{
    FILE *fin = fopen(inputFile, "rb");
    FILE *fout = fopen(encryptedFile, "wb");

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

void decryptFile(const char *inputFile, const char *encryptedFile, const char *key)
{
    FILE *fin = fopen(inputFile, "rb");
    FILE *fout = fopen(encryptedFile, "wb");

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

int compareFiles(const char *file1, const char *file2)
{
    FILE *fp1 = fopen(file1, "rb");
    FILE *fp2 = fopen(file2, "rb");

    if (fp1 == NULL || fp2 == NULL)
    {
        perror("Erro ao abrir os arquivos");
        return -1;
    }

    fseek(fp1, 0, SEEK_END);
    fseek(fp2, 0, SEEK_END);

    long fileSize1 = ftell(fp1);
    long fileSize2 = ftell(fp2);

    if (fileSize1 != fileSize2)
    {
        fclose(fp1);
        fclose(fp2);
        return 0; // Arquivos têm tamanhos diferentes
    }

    fseek(fp1, 0, SEEK_SET);
    fseek(fp2, 0, SEEK_SET);

    int byte1, byte2;
    int differentBytes = 0;

    while ((byte1 = fgetc(fp1)) != EOF && (byte2 = fgetc(fp2)) != EOF)
    {
        if (byte1 != byte2)
        {
            differentBytes = 1;
            break;
        }
    }

    fclose(fp1);
    fclose(fp2);

    return !differentBytes;
}

int main()
{
    const char *inputFile = "fractaljulia.bmp";
    const char *encryptedFile = "arquivo_encriptado.bmp";
    const char *decryptedFile = "arquivo_desencriptado.bmp";

    const char *key = "chave123";

    encryptFile(inputFile, encryptedFile, key);

    decryptFile(encryptedFile, decryptedFile, key);

    int isEncrypted = compareFiles(inputFile, encryptedFile);

    if (isEncrypted)
    {
        printf("O arquivo foi encriptado corretamente.\n");
    }
    else
    {
        printf("O arquivo não foi encriptado corretamente.\n");
    }

    return 0;
}
