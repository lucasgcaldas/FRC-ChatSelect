#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#pragma pack(1)  // Empacotamento de 1 byte

typedef struct {
    unsigned char type[2];
    unsigned int size;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int offset;
} BMPHeader;

typedef struct {
    unsigned int size;
    int width;
    int height;
    unsigned short planes;
    unsigned short bpp;
    unsigned int compression;
    unsigned int imageSize;
    int xResolution;
    int yResolution;
    unsigned int colors;
    unsigned int importantColors;
} BMPInfoHeader;

int isPrime(int num) {
    if (num < 2) {
        return 0;
    }

    for (int i = 2; i <= sqrt(num); i++) {
        if (num % i == 0) {
            return 0;
        }
    }

    return 1;
}

void readPrimesFromFile(const char* filename, int* p, int* q) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }

    fscanf(file, "%d#%d", p, q);
    fclose(file);
}

void saveToFile(const char* filename, int num) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }

    fprintf(file, "%d", num);
    fclose(file);
}

int gcd(int a, int b) {
    if (b == 0) {
        return a;
    }

    return gcd(b, a % b);
}

int modInverse(int a, int m) {
    a = a % m;

    for (int x = 1; x < m; x++) {
        if ((a * x) % m == 1) {
            return x;
        }
    }

    return 1;
}

void createKeys(int p, int q) {
    int n = p * q;
    int phi = (p - 1) * (q - 1);

    int e;
    for (e = 2; e < phi; e++) {
        if (gcd(e, phi) == 1) {
            break;
        }
    }

    int d = modInverse(e, phi);

    saveToFile("chave.pub", e);
    saveToFile("chave.priv", d);
}

void encryptFile(const char* inputFile, const char* outputFile, int key) {
    FILE* input = fopen(inputFile, "rb");
    FILE* output = fopen(outputFile, "wb");

    if (input == NULL || output == NULL) {
        printf("Erro ao abrir os arquivos.\n");
        return;
    }

    BMPHeader bmpHeader;
    BMPInfoHeader bmpInfoHeader;

    fread(&bmpHeader, sizeof(BMPHeader), 1, input);
    fread(&bmpInfoHeader, sizeof(BMPInfoHeader), 1, input);

    fwrite(&bmpHeader, sizeof(BMPHeader), 1, output);
    fwrite(&bmpInfoHeader, sizeof(BMPInfoHeader), 1, output);

    unsigned char pixel;
    while (fread(&pixel, sizeof(unsigned char), 1, input) == 1) {
        pixel = pixel ^ key;
        fwrite(&pixel, sizeof(unsigned char), 1, output);
    }

    fclose(input);
    fclose(output);
}

void decryptFile(const char* inputFile, const char* outputFile, int key) {
    encryptFile(inputFile, outputFile, key); // A desencriptação é o mesmo processo da encriptação
}

int main() {
    int p, q;

    readPrimesFromFile("primes.txt", &p, &q);
    createKeys(p, q);

    encryptFile("fractaljulia.bmp", "encrypted.bmp", p * q); // Use a chave pública adequada
    decryptFile("encrypted.bmp", "decrypted.bmp", p * q); // Use a chave privada adequada

    return 0;
}
