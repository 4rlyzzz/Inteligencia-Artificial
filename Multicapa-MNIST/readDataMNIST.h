#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>

void loadImages(const std::string& path, std::vector<float>& vectorImage, int totalImages, int pixelImage) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "error al abrir el archivo de imagenes: " << path << "\n";
        exit(1);
    }
    file.seekg(16);
    for (int i = 0; i < totalImages * pixelImage; ++i) {
        unsigned char pixel = 0;
        file.read((char*)&pixel, 1);
        vectorImage.push_back((float)pixel / 255.0f);
    }
}

void loadLabels(const std::string& path, std::vector<int>& vectorLabels, int totalLabels) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "error al abrir el archivo de labels: " << path << "\n";
        exit(1);
    }
    file.seekg(8);
    for (int i = 0; i < totalLabels; ++i) {
        unsigned char etiqueta = 0;
        file.read((char*)&etiqueta, 1);
        vectorLabels.push_back((int)etiqueta);
    }
}