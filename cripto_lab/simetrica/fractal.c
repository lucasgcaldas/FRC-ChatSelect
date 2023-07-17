#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define OUTFILE "fractaljulia.bmp"

int compute_julia_pixel(int x, int y, int largura, int altura, float tint_bias, unsigned char *rgb)
{
    // Check coordinates
    if ((x < 0) || (x >= largura) || (y < 0) || (y >= altura))
    {
        fprintf(stderr, "Coordenadas de pixel inválidas (%d,%d) em uma imagem %d x %d\n", x, y, largura, altura);
        return -1;
    }
    // "Zoom in" para uma visualização agradável do conjunto de Julia
    float X_MIN = -1.6, X_MAX = 1.6, Y_MIN = -0.9, Y_MAX = +0.9;
    float float_y = (Y_MAX - Y_MIN) * (float)y / altura + Y_MIN;
    float float_x = (X_MAX - X_MIN) * (float)x / largura + X_MIN;
    // Ponto que define o conjunto de Julia
    float julia_real = -0.79;
    float julia_img = 0.15;
    // Número máximo de iterações
    int max_iter = 300;
    // Calcular a convergência da série complexa
    float real = float_y, img = float_x;
    int num_iter = max_iter;
    while ((img * img + real * real < 2 * 2) && (num_iter > 0))
    {
        float xtemp = img * img - real * real + julia_real;
        real = 2 * img * real + julia_img;
        img = xtemp;
        num_iter--;
    }

    // Pintar o pixel com base no número de iterações usadas, usando algumas cores diferentes
    float color_bias = (float)num_iter / max_iter;
    rgb[0] = (num_iter == 0 ? 200 : -500.0 * pow(tint_bias, 1.2) * pow(color_bias, 1.6));
    rgb[1] = (num_iter == 0 ? 100 : -255.0 * pow(color_bias, 0.3));
    rgb[2] = (num_iter == 0 ? 100 : 255 - 255.0 * pow(tint_bias, 1.2) * pow(color_bias, 3.0));

    return 0;
} /* fim compute julia pixel */

int write_bmp_header(FILE *f, int largura, int altura)
{
    unsigned int row_size_in_bytes = largura * 3 +
                                     ((largura * 3) % 4 == 0 ? 0 : (4 - (largura * 3) % 4));

    // Define todos os campos do cabeçalho BMP
    char id[3] = "BM";
    unsigned int filesize = 54 + (int)(row_size_in_bytes * altura * sizeof(char));
    short reserved[2] = {0, 0};
    unsigned int offset = 54;

    unsigned int size = 40;
    unsigned short planes = 1;
    unsigned short bits = 24;
    unsigned int compression = 0;
    unsigned int image_size = largura * altura * 3 * sizeof(char);
    int x_res = 0;
    int y_res = 0;
    unsigned int ncolors = 0;
    unsigned int importantcolors = 0;

    // Escreve os bytes no arquivo, mantendo o controle do
    // número de "objetos" escritos com sucesso
    size_t ret = 0;
    ret += fwrite(id, sizeof(char), 2, f);
    ret += fwrite(&filesize, sizeof(int), 1, f);
    ret += fwrite(reserved, sizeof(short), 2, f);
    ret += fwrite(&offset, sizeof(int), 1, f);
    ret += fwrite(&size, sizeof(int), 1, f);
    ret += fwrite(&largura, sizeof(int), 1, f);
    ret += fwrite(&altura, sizeof(int), 1, f);
    ret += fwrite(&planes, sizeof(short), 1, f);
    ret += fwrite(&bits, sizeof(short), 1, f);
    ret += fwrite(&compression, sizeof(int), 1, f);
    ret += fwrite(&image_size, sizeof(int), 1, f);
    ret += fwrite(&x_res, sizeof(int), 1, f);
    ret += fwrite(&y_res, sizeof(int), 1, f);
    ret += fwrite(&ncolors, sizeof(int), 1, f);
    ret += fwrite(&importantcolors, sizeof(int), 1, f);

    // Sucesso significa que foram escritos 17 "objetos" com sucesso
    return (ret != 17);
} /* fim write bmp-header */

int main(int argc, char *argv[])
{
    int n;
    int area = 0, largura = 0, altura = 0, local_i = 0;
    FILE *output_file;
    unsigned char *pixel_array, *rgb;

    if ((argc <= 1) || (atoi(argv[1]) < 1))
    {
        fprintf(stderr, "Digite um valor inteiro positivo 'N' como argumento!\n");
        return -1;
    }
    n = atoi(argv[1]);
    altura = n;
    largura = 2 * n;
    area = altura * largura * 3;
    // Aloca memória para o array de pixels
    pixel_array = (unsigned char *)calloc(area, sizeof(unsigned char));
    rgb = (unsigned char *)calloc(3, sizeof(unsigned char));
    printf("Calculando linhas de pixel de %d a %d, para uma área total de %d\n", 0, n - 1, area);

    for (int i = 0; i < altura; i++)
    {
        for (int j = 0; j < largura; j++)
        {
            compute_julia_pixel(j, i, largura, altura, 1.0, rgb);
            pixel_array[local_i++] = rgb[0];
            pixel_array[local_i++] = rgb[1];
            pixel_array[local_i++] = rgb[2];
        }
    }

    // Libera memória do array de pixels
    free(rgb);

    // Escreve o cabeçalho do arquivo
    output_file = fopen(OUTFILE, "wb");
    if (output_file == NULL)
    {
        fprintf(stderr, "Erro ao abrir o arquivo de saída\n");
        free(pixel_array);
        return -1;
    }
    if (write_bmp_header(output_file, largura, altura) != 0)
    {
        fprintf(stderr, "Erro ao escrever o cabeçalho do arquivo\n");
        fclose(output_file);
        free(pixel_array);
        return -1;
    }
    // Escreve o array no arquivo
    if (fwrite(pixel_array, sizeof(unsigned char), area, output_file) != area)
    {
        fprintf(stderr, "Erro ao escrever o array de pixels no arquivo\n");
        fclose(output_file);
        free(pixel_array);
        return -1;
    }
    fclose(output_file);
    free(pixel_array);
    printf("Arquivo BMP gerado com sucesso: %s\n", OUTFILE);
    return 0;
}