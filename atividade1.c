#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    unsigned char r, g, b;
} Pixel;

typedef struct {
    int width, height;
    Pixel *data;
} Image;

Image *readPPM(const char *filename);
void writePPM(const char *filename, Image *img);
Image *convertToGray(Image *img);
Image *convertToNegative(Image *img);
Image *convertToXRay(Image *img, float intensityFactor);
Image *convertToSepia(Image *img, float fator, int tint);
Image *rotate90(Image *img);
Image *rotate180(Image *img);
void freeImage(Image *img);

int main() {
    char filename[100];
    char outputFilename[150];
    int choice;
    float intensityFactor = 1.5;
    Image *img = NULL, *processedImg = NULL;

    printf("Processador de Imagens PPM\n");
    printf("-------------------------\n");
    
    printf("Digite o nome da imagem PPM: ");
    scanf("%s", filename);
    
    if (strstr(filename, ".ppm") == NULL) {
        printf("Erro: O arquivo deve ter extensão .ppm\n");
        return 1;
    }

    img = readPPM(filename);
    if (img == NULL) {
        printf("Erro ao carregar a imagem.\n");
        return 1;
    }
    
    if (img->width < 400 || img->height < 400) {
        printf("Erro: A imagem deve ter tamanho mínimo de 400x400 pixels.\n");
        freeImage(img);
        return 1;
    }

    printf("\nOpções de processamento:\n");
    printf("1. Converter para tons de cinza\n");
    printf("2. Converter para negativo\n");
    printf("3. Converter para raio-X\n");
    printf("4. Converter para sépia (envelhecida)\n");
    printf("5. Rotacionar 90 graus\n");
    printf("6. Rotacionar 180 graus\n");
    printf("Escolha uma opção (1-6): ");
    scanf("%d", &choice);

    if (choice == 3) {
        printf("Digite o fator de intensidade para raio-X (1.0 a 2.0): ");
        scanf("%f", &intensityFactor);
        if (intensityFactor < 1.0) {
            intensityFactor = 1.0;
        } else if (intensityFactor > 2.0) {
            intensityFactor = 2.0;
        }
    }

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
            processedImg = convertToSepia(img, 0.1, 10);
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

    strcat(outputFilename, filename);
    
    writePPM(outputFilename, processedImg);
    printf("Imagem processada salva como: %s\n", outputFilename);

    freeImage(img);
    freeImage(processedImg);

    return 0;
}

Image *readPPM(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;

    char formato[3];
    int maxval;
    Image *img = (Image *)malloc(sizeof(Image));

    if (fscanf(file, "%2s", formato) != 1 || strcmp(formato, "P3") != 0) {
        fprintf(stderr, "Formato inválido (esperado P3)\n");
        free(img);
        fclose(file);
        return NULL;
    }

    if (fscanf(file, "%d %d", &img->width, &img->height) != 2 ||
        fscanf(file, "%d", &maxval) != 1) {
        fprintf(stderr, "Erro ao ler dimensões ou maxval\n");
        free(img);
        fclose(file);
        return NULL;
    }

    img->data = (Pixel *)malloc(img->width * img->height * sizeof(Pixel));
    for (int i = 0; i < img->width * img->height; i++) {
        int r, g, b;
        if (fscanf(file, "%d %d %d", &r, &g, &b) != 3) {
            fprintf(stderr, "Erro ao ler pixel #%d\n", i);
            free(img->data);
            free(img);
            fclose(file);
            return NULL;
        }
        img->data[i].r = (unsigned char)r;
        img->data[i].g = (unsigned char)g;
        img->data[i].b = (unsigned char)b;
    }

    fclose(file);
    return img;
}

void writePPM(const char *filename, Image *img) {
    FILE *file = fopen(filename, "w");
    if (!file) return;

    fprintf(file, "P3\n%d %d\n255\n", img->width, img->height);
    for (int i = 0; i < img->width * img->height; i++) {
        fprintf(file, "%u %u %u\n",
                img->data[i].r,
                img->data[i].g,
                img->data[i].b);
    }

    fclose(file);
}

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

Image *convertToSepia(Image *img, float fator, int tint) {
    Image *sepia = (Image *)malloc(sizeof(Image));
    sepia->width = img->width;
    sepia->height = img->height;
    sepia->data = (Pixel *)malloc(sepia->width * sepia->height * sizeof(Pixel));

    for (int i = 0; i < img->width * img->height; i++) {
        int r = (int)(img->data[i].r * (1.0 + fator)) + tint;
        int g = (int)(img->data[i].g * (1.0 + fator)) + tint;
        int b = (int)(img->data[i].b * (1.0 - fator)) - tint;

        sepia->data[i].r = (r > 255) ? 255 : (r < 0 ? 0 : r);
        sepia->data[i].g = (g > 255) ? 255 : (g < 0 ? 0 : g);
        sepia->data[i].b = (b > 255) ? 255 : (b < 0 ? 0 : b);
    }

    return sepia;
}


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

void freeImage(Image *img) {
    if (img) {
        if (img->data) {
            free(img->data);
        }
        free(img);
    }
}