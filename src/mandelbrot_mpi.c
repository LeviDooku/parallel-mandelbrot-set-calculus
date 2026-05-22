#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define WIDTH 1920
#define HEIGHT 1080

#define X_MIN -2.0
#define X_MAX 1.0
#define Y_MIN -1.5
#define Y_MAX 1.5

int mandelbrot(double cx, double cy, int max_iter) {
    double zx = 0.0;
    double zy = 0.0;
    double zx2 = 0.0;
    double zy2 = 0.0;
    int iter = 0;

    while (zx2 + zy2 <= 4.0     && iter < max_iter) {
        zy = 2.0 * zx * zy + cy;
        zx = zx2 - zy2 + cx;

        zx2 = zx * zx;
        zy2 = zy * zy;

        iter++;
    }

    return iter;
}

void save_pgm(const char *filename, int *image, int width, int height, int max_iter) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: no se pudo abrir el archivo %s\n", filename);
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
    int rango, size;
    int max_iter;
    int guardar_imagen = 0;

    MPI_Init(&argc, &argv); //Inicializar entorno MPI

    MPI_Comm_rank(MPI_COMM_WORLD, &rango); //Identificar proceso
    MPI_Comm_size(MPI_COMM_WORLD, &size); //Número total de procesos

    if (argc < 2 || argc > 3) {
        if (rango == 0) {
            fprintf(stderr, "Uso: %s <max_iter> [save]\n", argv[0]);
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    max_iter = atoi(argv[1]);

    if (max_iter <= 0) {
        if (rango == 0) {
            fprintf(stderr, "Error: max_iter debe ser > 0\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    //Permitir elegir o no guardar la imagen
    if (argc == 3 && strcmp(argv[2], "save") == 0)
        guardar_imagen = 1;


    //Reparto cíclico de las filas a calcular
    int local_rows = 0;
    for (int row = rango; row < HEIGHT; row += size) {
        local_rows++;
    }

    //Memoria local. Cada proceso reserva solo para sus filas
    int local_elems = local_rows * WIDTH;
    int *local_image = (int *) malloc(local_elems * sizeof(int));
    if (local_image == NULL) {
        fprintf(stderr, "Proceso %d: error al reservar memoria local\n", rango);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    //Cálculo en local, prácticamente igual que en secuencial, pero cada proceso recorre solo sus filas
    int local_row_index = 0;
    for (int row = rango; row < HEIGHT; row += size) {
        for (int col = 0; col < WIDTH; col++) {
            double cx = X_MIN + (X_MAX - X_MIN) * col / (WIDTH - 1);
            double cy = Y_MIN + (Y_MAX - Y_MIN) * row / (HEIGHT - 1);

            local_image[local_row_index * WIDTH + col] = mandelbrot(cx, cy, max_iter);
        }
        local_row_index++;
    }

    //Preparación para unir resultados
    int *recvcounts = NULL;
    int *displs = NULL;
    int *gathered = NULL;
    int *image = NULL;

    //El proceso 0 reserva memoria 
    if (rango == 0) {
        recvcounts = (int *) malloc(size * sizeof(int));
        displs = (int *) malloc(size * sizeof(int));

        if (recvcounts == NULL || displs == NULL) {
            fprintf(stderr, "Proceso 0: error al reservar memoria auxiliar\n");
            free(local_image);
            free(recvcounts);
            free(displs);
            MPI_Finalize();
            return EXIT_FAILURE;
        }
    }

    //Cada proceso envía cuántos enteros ha calculado
    MPI_Gather(&local_elems, 1, MPI_INT, recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //El proceso 0 determina donde empieza el bloque enviado por cada proceso y reserva memoria global
    if (rango == 0) {
        displs[0] = 0;
        for (int p = 1; p < size; p++) {
            displs[p] = displs[p - 1] + recvcounts[p - 1];
        }

        gathered = (int *) malloc(WIDTH * HEIGHT * sizeof(int));
        image = (int *) malloc(WIDTH * HEIGHT * sizeof(int));

        if (gathered == NULL || image == NULL) {
            fprintf(stderr, "Proceso 0: error al reservar memoria global\n");
            free(local_image);
            free(recvcounts);
            free(displs);
            free(gathered);
            free(image);
            MPI_Finalize();
            return EXIT_FAILURE;
        }
    }

    //Se recolectan los datos, los local_image en gathered. Aún no están ordenados
    MPI_Gatherv(local_image, local_elems, MPI_INT, gathered, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    //En base al proceso cíclico de repartición, se deshace y se reconstruye la imagen
    if (rango == 0) {
        for (int p = 0; p < size; p++) {
            int rows_p = recvcounts[p] / WIDTH;

            for (int lr = 0; lr < rows_p; lr++) {
                int global_row = p + lr * size;
                int src_offset = displs[p] + lr * WIDTH;
                int dst_offset = global_row * WIDTH;

                memcpy(&image[dst_offset], &gathered[src_offset], WIDTH * sizeof(int));
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end = MPI_Wtime();

    if (rango == 0) {
        double elapsed_time = end - start;

        //Muestra los datos de forma cómoda para analizarlos
        printf("%d %d %.6f\n", max_iter, size, elapsed_time);

        if (guardar_imagen)
            save_pgm("img/img_res_mpi.pgm", image, WIDTH, HEIGHT, max_iter);
    }

    free(local_image);

    if (rango == 0) {
        free(recvcounts);
        free(displs);
        free(gathered);
        free(image);
    }

    MPI_Finalize();
    return 0;
}
