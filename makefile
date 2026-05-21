CC = gcc
MPICC = mpicc

CFLAGS = -Wall -std=c11
MPICFLAGS = -Wall -std=c11
LDFLAGS = -lm

BIN = bin
DATA = dat
SRC = src
IMG = img
PLOT = plot

SEQ_SRC = $(SRC)/mandelbrot_seq.c
SEQ_BIN = $(BIN)/mandelbrot_seq

MPI_SRC = $(SRC)/mandelbrot_mpi.c
MPI_BIN = $(BIN)/mandelbrot_mpi

SEQ_DATA = $(DATA)/seq_data.dat
SEQ_IMG = $(IMG)/img_res_seq.pgm

MPI_DATA = $(DATA)/mpi_data.dat
MPI_IMG = $(IMG)/img_res_mpi.pgm

ITER = 250 500 1000 2000 3000 5000 7500 10000 15000	#Adjustable
PROCCESS = 2 4 6 8 					#Adjustable

.PHONY: all dirs try_seq try_parallel clean_data clean_img clean_bin  clean

all: dirs compile try_seq try_parallel

dirs: 
		mkdir -p $(BIN)
		mkdir -p $(DATA)
		mkdir -p $(IMG)
		mkdir -p $(PLOT)
		@echo "[+] Directories OK"

compile:
	$(MPICC) $(CFLAGS) -o $(MPI_BIN) $(MPI_SRC)
	$(CC) $(CFLAGS) -o $(SEQ_BIN) $(SEQ_SRC)
	@echo "[+] Compilation OK: $(MPI_BIN), $(SEQ_BIN) generated"

try_seq:
	@echo "nada aun"
try_parallel:
	@echo "nada aun"
	
clean_data:
		rm -rf $(DATA)
		rm -rf $(PLOT)
		@echo "[+] Deleted /$(DATA) and /$(PLOT)"
clean_img:
		rm -rf $(IMG)
		@echo "[+] Deleted /$(IMG)"
clean_bin:
		rm -rf $(BIN)
		@echo "[+] Deleted /$(BIN)"
#Está mal

clean: 
		clean_data
		clean_img
		clean_bin
