#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

bool is_prime(int number) {
    if (number < 2) {
        return false;
    }
    for (int i = 2; i * i <= number; i++) {
        if (number % i == 0) {
            return false;
        }
    }
    return true;
}

int generate_prime(int min, int max) {
    int number;
    do {
        number = min + rand() % (max - min + 1);
    } while (!is_prime(number));
    return number;
}

int main() {
    srand(time(NULL));

    int prime1, prime2;
    while (true) {
        prime1 = generate_prime(10000, 99999);
        prime2 = generate_prime(10000, 99999);
        if (prime1 != prime2) {
            break;
        }
    }

    FILE* file = fopen("primes.txt", "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    fprintf(file, "%d#%d", prime1, prime2);
    fclose(file);

    printf("Números primos gerados e salvos no arquivo \"primes.txt\" com sucesso!\n");

    return 0;
}