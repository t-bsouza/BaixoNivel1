#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Estrutura para armazenar um pixel RGB
typedef struct {
    unsigned char r, g, b;
} Pixel;

// Estrutura para armazenar a imagem
typedef struct {
    int width, height;
    Pixel *data;
} Image;

// Funções de manipulação de imagem
Image *readPPM(const char *filename);
void writePPM(const char *filename, Image *img);
Image *convertToGray(Image *img);
Image *convertToNegative(Image *img);
Image *convertToXRay(Image *img, float intensityFactor);
Image *convertToSepia(Image *img);
Image *rotate90(Image *img);
Image *rotate180(Image *img);
void freeImage(Image *img);

int main() {
    char filename[100];
    char outputFilename[150];
    int choice;
    float intensityFactor = 1.5; // Fator de intensidade inicial para raio-X
    Image *img = NULL, *processedImg = NULL;

    printf("Processador de Imagens PPM\n");
    printf("-------------------------\n");
    
    // Solicitar o nome da imagem
    printf("Digite o nome da imagem PPM (formato *.ppm): ");
    scanf("%s", filename);
    
    // Verificar se o nome termina com .ppm
    if (strstr(filename, ".ppm") == NULL) {
        printf("Erro: O arquivo deve ter extensão .ppm\n");
        return 1;
    }

    // Ler a imagem
    img = readPPM(filename);
    if (img == NULL) {
        printf("Erro ao carregar a imagem.\n");
        return 1;
    }
    
    // Verificar tamanho mínimo
    if (img->width < 400 || img->height < 400) {
        printf("Erro: A imagem deve ter tamanho mínimo de 400x400 pixels.\n");
        freeImage(img);
        return 1;
    }

    // Menu de opções
    printf("\nOpções de processamento:\n");
    printf("1. Converter para tons de cinza\n");
    printf("2. Converter para negativo\n");
    printf("3. Converter para raio-X\n");
    printf("4. Converter para sépia (envelhecida)\n");
    printf("5. Rotacionar 90 graus\n");
    printf("6. Rotacionar 180 graus\n");
    printf("Escolha uma opção (1-6): ");
    scanf("%d", &choice);

    // Se a opção for raio-X, perguntar pelo fator de intensidade
    if (choice == 3) {
        printf("Digite o fator de intensidade para raio-X (1.0 a 2.0): ");
        scanf("%f", &intensityFactor);
        if (intensityFactor < 1.0) {
            intensityFactor = 1.0;
        } else if (intensityFactor > 2.0) {
            intensityFactor = 2.0;
        }
    }

    // Processar conforme a escolha
    switch (choice) {
        case 1:
            processedImg = convertToGray(img);
            strcpy(outputFilename, "gray_");
            break;
        case 2:
            processedImg = convertToNegative(img);
            strcpy(outputFilename, "negative_");
            break;
        case 3:
            processedImg = convertToXRay(img, intensityFactor);
            strcpy(outputFilename, "xray_");
            break;
        case 4:
            processedImg = convertToSepia(img);
            strcpy(outputFilename, "sepia_");
            break;
        case 5:
            processedImg = rotate90(img);
            strcpy(outputFilename, "rot90_");
            break;
        case 6:
            processedImg = rotate180(img);
            strcpy(outputFilename, "rot180_");
            break;
        default:
            printf("Opção inválida.\n");
            freeImage(img);
            return 1;
    }

    // Criar nome do arquivo de saída
    strcat(outputFilename, filename);
    
    // Salvar imagem processada
    writePPM(outputFilename, processedImg);
    printf("Imagem processada salva como: %s\n", outputFilename);

    // Liberar memória
    freeImage(img);
    freeImage(processedImg);

    return 0;
}

// Função para ler uma imagem PPM
Image *readPPM(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erro ao abrir o arquivo %s\n", filename);
        return NULL;
    }

    char magic[3];
    int maxval;
    Image *img = (Image *)malloc(sizeof(Image));
    
    // Ler cabeçalho
    fscanf(file, "%2s", magic);
    if (strcmp(magic, "P6") != 0) {
        printf("Formato PPM inválido (deve ser P6)\n");
        fclose(file);
        free(img);
        return NULL;
    }
    
    fscanf(file, "%d %d", &img->width, &img->height);
    fscanf(file, "%d", &maxval);
    fgetc(file); // Consumir o caractere de nova linha
    
    // Alocar memória para os pixels
    img->data = (Pixel *)malloc(img->width * img->height * sizeof(Pixel));
    
    // Ler dados da imagem
    fread(img->data, sizeof(Pixel), img->width * img->height, file);
    
    fclose(file);
    return img;
}

// Função para salvar uma imagem PPM
void writePPM(const char *filename, Image *img) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Erro ao criar o arquivo %s\n", filename);
        return;
    }
    
    // Escrever cabeçalho
    fprintf(file, "P6\n%d %d\n255\n", img->width, img->height);
    
    // Escrever dados da imagem
    fwrite(img->data, sizeof(Pixel), img->width * img->height, file);
    
    fclose(file);
}

// Converter para tons de cinza
Image *convertToGray(Image *img) {
    Image *gray = (Image *)malloc(sizeof(Image));
    gray->width = img->width;
    gray->height = img->height;
    gray->data = (Pixel *)malloc(gray->width * gray->height * sizeof(Pixel));
    
    for (int i = 0; i < img->width * img->height; i++) {
        unsigned char grayValue = (unsigned char)(0.299 * img->data[i].r + 
                                                 0.587 * img->data[i].g + 
                                                 0.114 * img->data[i].b);
        gray->data[i].r = gray->data[i].g = gray->data[i].b = grayValue;
    }
    
    return gray;
}

// Converter para negativo
Image *convertToNegative(Image *img) {
    Image *neg = (Image *)malloc(sizeof(Image));
    neg->width = img->width;
    neg->height = img->height;
    neg->data = (Pixel *)malloc(neg->width * neg->height * sizeof(Pixel));
    
    for (int i = 0; i < img->width * img->height; i++) {
        neg->data[i].r = 255 - img->data[i].r;
        neg->data[i].g = 255 - img->data[i].g;
        neg->data[i].b = 255 - img->data[i].b;
    }
    
    return neg;
}

// Converter para efeito raio-X
Image *convertToXRay(Image *img, float intensityFactor) {
    Image *xray = (Image *)malloc(sizeof(Image));
    xray->width = img->width;
    xray->height = img->height;
    xray->data = (Pixel *)malloc(xray->width * xray->height * sizeof(Pixel));
    
    for (int i = 0; i < img->width * img->height; i++) {
        unsigned char grayValue = (unsigned char)(0.299 * img->data[i].r + 
                                                 0.587 * img->data[i].g + 
                                                 0.114 * img->data[i].b);
        unsigned char inverted = 255 - (unsigned char)(pow((float)grayValue / 255.0, intensityFactor) * 255.0);
        xray->data[i].r = xray->data[i].g = xray->data[i].b = inverted;
    }
    
    return xray;
}

// Converter para sépia (envelhecida)
Image *convertToSepia(Image *img) {
    Image *sepia = (Image *)malloc(sizeof(Image));
    sepia->width = img->width;
    sepia->height = img->height;
    sepia->data = (Pixel *)malloc(sepia->width * sepia->height * sizeof(Pixel));
    
    for (int i = 0; i < img->width * img->height; i++) {
        int tr = (int)(0.393 * img->data[i].r + 0.769 * img->data[i].g + 0.189 * img->data[i].b);
        int tg = (int)(0.349 * img->data[i].r + 0.686 * img->data[i].g + 0.168 * img->data[i].b);
        int tb = (int)(0.272 * img->data[i].r + 0.534 * img->data[i].g + 0.131 * img->data[i].b);
        
        sepia->data[i].r = (tr > 255) ? 255 : tr;
        sepia->data[i].g = (tg > 255) ? 255 : tg;
        sepia->data[i].b = (tb > 255) ? 255 : tb;
    }
    
    return sepia;
}

// Rotacionar 90 graus no sentido horário
Image *rotate90(Image *img) {
    Image *rotated = (Image *)malloc(sizeof(Image));
    rotated->width = img->height;
    rotated->height = img->width;
    rotated->data = (Pixel *)malloc(rotated->width * rotated->height * sizeof(Pixel));
    
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            rotated->data[x * rotated->width + (rotated->width - 1 - y)] = img->data[y * img->width + x];
        }
    }
    
    return rotated;
}

// Rotacionar 180 graus
Image *rotate180(Image *img) {
    Image *rotated = (Image *)malloc(sizeof(Image));
    rotated->width = img->width;
    rotated->height = img->height;
    rotated->data = (Pixel *)malloc(rotated->width * rotated->height * sizeof(Pixel));
    
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            rotated->data[(img->height - 1 - y) * img->width + (img->width - 1 - x)] = img->data[y * img->width + x];
        }
    }
    
    return rotated;
}

// Liberar memória da imagem
void freeImage(Image *img) {
    if (img) {
        if (img->data) {
            free(img->data);
        }
        free(img);
    }
}
