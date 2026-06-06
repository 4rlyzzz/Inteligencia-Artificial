#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <cuda_runtime.h>
#include "kernel.h"
#include "readDataMNIST.h"

const int PIXELS = 784; // 28x28 pixeles por imagen
const int CLASSES = 10; // 0 al 9
const int BATCH = 100; // 10 digitos x 10 imagenes
const int TRAIN_TOTAL = 60000;
const int TEST_TOTAL = 10000;
const int EPOCAS = 10;
const float LR = 0.01f; // tasa de aprendizaje

std::mt19937 rng(static_cast<unsigned>(time(nullptr)));

int randInt(int lo, int hi) {
    return std::uniform_int_distribution<int>(lo, hi - 1)(rng);
}
float randFloat(float lo, float hi) {
    return std::uniform_real_distribution<float>(lo, hi)(rng);
}

// valor mas alto 
int argmax(const float* row, int n) {
    int best = 0;
    for (int i = 1; i < n; ++i)
        if (row[i] > row[best]) best = i;
    return best;
}

void inicializarGPU(float*& gpuWeights, float*& gpuBatchImages, float*& gpuExpectedOutput, float*& gpuPredictions, const std::vector<float>& weights) {
    cudaMalloc(&gpuWeights, CLASSES * 785 * sizeof(float));
    cudaMalloc(&gpuBatchImages, BATCH * PIXELS * sizeof(float));
    cudaMalloc(&gpuExpectedOutput, BATCH * CLASSES * sizeof(float));
    cudaMalloc(&gpuPredictions, BATCH * CLASSES * sizeof(float));

    cudaMemcpy(gpuWeights, weights.data(), weights.size() * sizeof(float), cudaMemcpyHostToDevice);
}

void ejecutarEpoca(const std::vector<float>& trainImages, const std::vector<std::vector<int>>& byDigit, std::vector<float>& batchImages, std::vector<float>& batchExpectedOutput, 
    float* gpuBatchImages, float* gpuExpectedOutput, float* gpuWeights, float* gpuPredictions, int epochNum) {

    // batch - 10 imagenes por digito
    std::fill(batchExpectedOutput.begin(), batchExpectedOutput.end(), 0.0f);
    int pos = 0;
    for (int digit = 0; digit < CLASSES; ++digit) {
        for (int s = 0; s < 10; ++s, ++pos) {
            int idx = byDigit[digit][randInt(0, byDigit[digit].size())];

            std::copy_n(&trainImages[idx * PIXELS], PIXELS, &batchImages[pos * PIXELS]); 

            batchExpectedOutput[pos * CLASSES + digit] = 1.0f; // 1 solo en la pos correcta
        }
    }

    cudaMemcpy(gpuBatchImages, batchImages.data(), batchImages.size() * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(gpuExpectedOutput, batchExpectedOutput.data(), batchExpectedOutput.size() * sizeof(float), cudaMemcpyHostToDevice);

    // 64 hilos por bloque
    dim3 threads(64);
    dim3 blocks((BATCH + 63) / 64, CLASSES);    

    forward << <blocks, threads >> > (gpuBatchImages, gpuWeights, gpuPredictions, BATCH, CLASSES, 1);
    cudaDeviceSynchronize();


    // PRINT POR EPOCA
    std::vector<float> batchPredictions(BATCH * CLASSES);
    cudaMemcpy(batchPredictions.data(), gpuPredictions, batchPredictions.size() * sizeof(float), cudaMemcpyDeviceToHost);

    std::cout << "\n----------- EPOCA " << epochNum << " -----------\n";
    for (int digit = 0; digit < CLASSES; ++digit) {
        int i = digit; 
        int real = argmax(&batchExpectedOutput[i * CLASSES], CLASSES);
        int pred = argmax(&batchPredictions[i * CLASSES], CLASSES);


        std::cout << "  ejemplo " << i << "  real=" << real<< "  pred=" << pred << "  ";
        if (pred == real) std::cout << "ok";
        else std::cout << "mal";
        std::cout << "\n";
    }
    // PRINT POR EPOCA


    backward << <blocks, threads >> > (gpuBatchImages, gpuExpectedOutput, gpuPredictions, gpuWeights, LR, BATCH, CLASSES);
    cudaDeviceSynchronize();
}

std::vector<float> evaluarModelo(const std::vector<float>& testImages, float* gpuWeights) {

    float* gpuTestImages, * gpuTestPredictions;

    cudaMalloc(&gpuTestImages, TEST_TOTAL * PIXELS * sizeof(float));
    cudaMalloc(&gpuTestPredictions, TEST_TOTAL * CLASSES * sizeof(float));

    cudaMemcpy(gpuTestImages, testImages.data(), testImages.size() * sizeof(float), cudaMemcpyHostToDevice);

    dim3 testBlocks((TEST_TOTAL + 255) / 256, CLASSES);

    forward << <testBlocks, dim3(256) >> > (gpuTestImages, gpuWeights, gpuTestPredictions, TEST_TOTAL, CLASSES, 0);
    cudaDeviceSynchronize();

    std::vector<float> testPredictions(TEST_TOTAL * CLASSES);
    cudaMemcpy(testPredictions.data(), gpuTestPredictions, testPredictions.size() * sizeof(float), cudaMemcpyDeviceToHost);

    cudaFree(gpuTestImages);
    cudaFree(gpuTestPredictions);

    return testPredictions;
}

void mostrarResultados(const std::vector<float>& testPredictions, const std::vector<int>& testLabels) {
    int errorCount = 0;
    for (int i = 0; i < TEST_TOTAL; ++i)
        if (argmax(&testPredictions[i * CLASSES], CLASSES) != testLabels[i])
            errorCount++;

    float accuracy = 100.0f * (TEST_TOTAL - errorCount) / TEST_TOTAL;
    std::cout << "\n----- resultados -----\n" << "errores:   " << errorCount << " / " << TEST_TOTAL << "\n" << "precision: " << accuracy << "%\n";
}

int main() {

    std::vector<float> trainImages, testImages;
    std::vector<int>   trainLabels, testLabels;

    std::cout << "cargando mnist...\n";
    loadImages("train-images.idx3-ubyte", trainImages, TRAIN_TOTAL, PIXELS);
    loadLabels("train-labels.idx1-ubyte", trainLabels, TRAIN_TOTAL);
    loadImages("t10k-images.idx3-ubyte", testImages, TEST_TOTAL, PIXELS);
    loadLabels("t10k-labels.idx1-ubyte", testLabels, TEST_TOTAL);

    // batches y pesos
    std::vector<std::vector<int>> byDigit(CLASSES);
    for (int i = 0; i < TRAIN_TOTAL; ++i)
        byDigit[trainLabels[i]].push_back(i);
    std::vector<float> weights(CLASSES * 785);
    for (float& w : weights)
        w = randFloat(-0.01f, 0.01f);

    // init
    float* gpuWeights, * gpuBatchImages, * gpuExpectedOutput, * gpuPredictions;
    inicializarGPU(gpuWeights, gpuBatchImages, gpuExpectedOutput, gpuPredictions, weights);

    // entrenamiento
    std::vector<float> batchImages(BATCH * PIXELS);
    std::vector<float> batchExpectedOutput(BATCH * CLASSES, 0.0f);

    std::cout << "entrenando...\n";
    for (int epoch = 0; epoch < EPOCAS; ++epoch) {
        ejecutarEpoca(trainImages, byDigit, batchImages, batchExpectedOutput, gpuBatchImages, gpuExpectedOutput, gpuWeights, gpuPredictions, epoch + 1);

        if ((epoch + 1) % 100 == 0)
            std::cout << "epoca " << epoch + 1 << " / " << EPOCAS << "\n";
    }

    // evaluacion
    std::cout << "\nevaluando con " << TEST_TOTAL << " imagenes...\n";
    auto testPredictions = evaluarModelo(testImages, gpuWeights);
    mostrarResultados(testPredictions, testLabels);


    cudaFree(gpuWeights);
    cudaFree(gpuBatchImages);
    cudaFree(gpuExpectedOutput);
    cudaFree(gpuPredictions);

    return 0;
}