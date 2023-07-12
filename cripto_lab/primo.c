#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

bool isPrime(int number)
{
    if (number < 2)
        return false;

    for (int i = 2; i <= number / 2; ++i)
    {
        if (number % i == 0)
        {
            return false;
        }
    }

    return true;
}

int generateRandomPrime()
{
    int min = 10000;  // Menor valor de 5 dígitos
    int max = 999999; // Maior valor de 6 dígitos

    srand(time(NULL)); // Inicializa a semente aleatória

    while (true)
    {
        int randomNumber = (rand() % (max - min + 1)) + min;

        if (isPrime(randomNumber))
        {
            return randomNumber;
        }
    }
}

int main()
{
    int randomNumber = generateRandomPrime();
    int randomNumber2 = generateRandomPrime();

    printf("%d#%d", randomNumber, randomNumber2);

    return 0;
}