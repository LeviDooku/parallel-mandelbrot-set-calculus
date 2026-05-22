#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//Res. imagen de salida

#define WIDTH 1920
#define HEIGHT 1080

//Rango del plano complejo que se quiere representar

#define X_MIN -2.0
#define X_MAX 1.0
#define Y_MIN -1.5
#define Y_MAX 1.5

//Función que calcula cuántas iteraciones tarda un punto complejo en 
//"escapar" del conjunto.

int mandelbrot(double cx, double cy, int max_iter) {
    double zx = 0.0;
    double zy = 0.0;
    double zx2 = 0.0;
    double zy2 = 0.0;
    int iter = 0;

    while (zx2 + zy2 <= 4.0 && iter < max_iter) {
        zy = 2.0 * zx * zy + cy;
        zx = zx2 - zy2 + cx;

        zx2 = zx * zx;
        zy2 = zy * zy;

        iter++;
    }

    return iter;
}

//Guardar la salida en PGM

void save_pgm(const char *filename, int *image, int width, int height, int max_iter) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: could not open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fprintf(file, "P2\n");
    fprintf(file, "%d %d\n", width, height);
    fprintf(file, "255\n");

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int iter = image[i * width + j];
            int color = (iter == max_iter) ? 0 : (255 * iter / max_iter);
            fprintf(file, "%d ", color);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    int save_img = 0;

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <max_iter> [save]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int max_iter = atoi(argv[1]);

    if(max_iter <= 0)
        fprintf(stderr, "Error: max_iter must be > 0\n");

    if (argc == 3 && strcmp(argv[2], "save") == 0)
        save_img = 1;

    int *image = NULL;
    clock_t start, end;
    double elapsed_time;

    image = (int *) malloc(WIDTH * HEIGHT * sizeof(int));
    if (image == NULL) {
        fprintf(stderr, "Error: memory allocation failed\n");
        return EXIT_FAILURE;
    }

    start = clock();

    //Bucle donde está la carga computacional
    //Se recorre la imagen, cada píxel se convierte en un cx, cy del plano complejo
    //Se calcula cuántas iteraciones necesita el punto (función mandelbrot)

    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            double cx = X_MIN + (X_MAX - X_MIN) * col / (WIDTH - 1);
            double cy = Y_MIN + (Y_MAX - Y_MIN) * row / (HEIGHT - 1);

            image[row * WIDTH + col] = mandelbrot(cx, cy, max_iter);
        }
    }

    end = clock();

    elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;

    //Salida adecuada para analizar datos
    printf("%d %.6f\n", max_iter, elapsed_time);

    if(save_img)
        save_pgm("img/img_res_seq.pgm", image, WIDTH, HEIGHT, max_iter);

    free(image);

    return 0;
}
