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
const float LR = 1; // 0.5f; // tasa de aprendizaje
const int HIDDEN = 128; // size capa oculta
const int BATCHES_EPOCH = 100; // TRAIN_TOTAL / BATCH; // 600 batches por epoca

std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
int randInt(int lo, int hi) { 
    return std::uniform_int_distribution<int>(lo, hi - 1)(rng); 
}
float randFloat(float lo, float hi) { 
    return std::uniform_real_distribution<float>(lo, hi)(rng); 
}

// indice del valor mas alto en un arreglo
int argmax(const float* row, int n) {
    int best = 0;
    for (int i = 1; i < n; ++i) if (row[i] > row[best]) best = i;
    return best;
}


void inicializarGPU(float*& gpuImages, float*& gpuHidden, float*& gpuPredictions, float*& gpuExpectedOutput, float*& gpuGradHidden,
    float*& gpuWeightsH, float*& gpuBiasH, float*& gpuWeightsO, float*& gpuBiasO,
    const std::vector<float>& w_h, const std::vector<float>& w_o) {

    cudaMalloc(&gpuImages, BATCH * PIXELS * sizeof(float));
    cudaMalloc(&gpuHidden, BATCH * HIDDEN * sizeof(float));
    cudaMalloc(&gpuPredictions, BATCH * CLASSES * sizeof(float));
    cudaMalloc(&gpuExpectedOutput, BATCH * CLASSES * sizeof(float));
    cudaMalloc(&gpuGradHidden, BATCH * HIDDEN * sizeof(float));
    cudaMalloc(&gpuWeightsH, HIDDEN * PIXELS * sizeof(float));
    cudaMalloc(&gpuBiasH, HIDDEN * sizeof(float));
    cudaMalloc(&gpuWeightsO, CLASSES * HIDDEN * sizeof(float));
    cudaMalloc(&gpuBiasO, CLASSES * sizeof(float));

    cudaMemcpy(gpuWeightsH, w_h.data(), w_h.size() * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(gpuWeightsO, w_o.data(), w_o.size() * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemset(gpuBiasH, 0, HIDDEN * sizeof(float));
    cudaMemset(gpuBiasO, 0, CLASSES * sizeof(float));
}

// 10 imagenes por digito
void armarBatch(const std::vector<float>& trainImages, const std::vector<std::vector<int>>& byDigit,
    std::vector<float>& batchImages, std::vector<float>& batchExpectedOutput) {

    std::fill(batchExpectedOutput.begin(), batchExpectedOutput.end(), 0.0f);
    int pos = 0;
    for (int digit = 0; digit < CLASSES; ++digit) {
        for (int s = 0; s < 10; ++s, ++pos) {
            int idx = byDigit[digit][randInt(0, (int)byDigit[digit].size())];
            std::copy_n(&trainImages[idx * PIXELS], PIXELS, &batchImages[pos * PIXELS]);
            batchExpectedOutput[pos * CLASSES + digit] = 1.0f; // 1 solo en la pos correcta
        }
    }
}

int ejecutarBatch(const std::vector<float>& trainImages, const std::vector<std::vector<int>>& byDigit,
    std::vector<float>& batchImages, std::vector<float>& batchExpectedOutput,
    float* gpuImages, float* gpuHidden, float* gpuPredictions, float* gpuExpectedOutput, float* gpuGradHidden,
    float* gpuWeightsH, float* gpuBiasH, float* gpuWeightsO, float* gpuBiasO,
    bool medirPrecision) {

    armarBatch(trainImages, byDigit, batchImages, batchExpectedOutput);

    cudaMemcpy(gpuImages, batchImages.data(), batchImages.size() * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(gpuExpectedOutput, batchExpectedOutput.data(), batchExpectedOutput.size() * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemset(gpuGradHidden, 0, BATCH * HIDDEN * sizeof(float));

    dim3 thr(64);
    dim3 blk_h((BATCH + 63) / 64, HIDDEN);
    dim3 blk_o((BATCH + 63) / 64, CLASSES);

    // forward pass
    forwardHidden << <blk_h, thr >> > (gpuImages, gpuWeightsH, gpuBiasH, gpuHidden, BATCH, PIXELS, HIDDEN);
    forwardOutput << <blk_o, thr >> > (gpuHidden, gpuWeightsO, gpuBiasO, gpuPredictions, BATCH, HIDDEN, CLASSES);

    // backward pass
    backwardOutput << <blk_o, thr >> > (gpuHidden, gpuExpectedOutput, gpuPredictions, gpuWeightsO, gpuBiasO, gpuGradHidden, BATCH, HIDDEN, CLASSES, LR);
    backwardHidden << <blk_h, thr >> > (gpuImages, gpuHidden, gpuGradHidden, gpuWeightsH, gpuBiasH, BATCH, PIXELS, HIDDEN, LR);

    if (!medirPrecision) return -1;

    cudaDeviceSynchronize();
    std::vector<float> batchPredictions(BATCH * CLASSES);
    cudaMemcpy(batchPredictions.data(), gpuPredictions, batchPredictions.size() * sizeof(float), cudaMemcpyDeviceToHost);

    int aciertos = 0;
    for (int i = 0; i < BATCH; ++i) {
        int real = argmax(&batchExpectedOutput[i * CLASSES], CLASSES);
        int pred = argmax(&batchPredictions[i * CLASSES], CLASSES);
        if (real == pred) aciertos++;
    }
    return aciertos;
}

// epoca completa y precision del ultimo batch
void ejecutarEpoca(const std::vector<float>& trainImages, const std::vector<std::vector<int>>& byDigit,
    std::vector<float>& batchImages, std::vector<float>& batchExpectedOutput,
    float* gpuImages, float* gpuHidden, float* gpuPredictions, float* gpuExpectedOutput, float* gpuGradHidden,
    float* gpuWeightsH, float* gpuBiasH, float* gpuWeightsO, float* gpuBiasO,
    int epochNum) {

    int aciertos_epoca = 0;
    for (int b = 0; b < BATCHES_EPOCH; ++b) {
        bool ultimoBatch = (b == BATCHES_EPOCH - 1);
        int aciertos = ejecutarBatch(trainImages, byDigit, batchImages, batchExpectedOutput,
            gpuImages, gpuHidden, gpuPredictions, gpuExpectedOutput, gpuGradHidden,
            gpuWeightsH, gpuBiasH, gpuWeightsO, gpuBiasO, ultimoBatch);
        if (ultimoBatch) aciertos_epoca = aciertos;
    }
    std::cout << "epoca " << epochNum << " precision batch: " << (100.0f * aciertos_epoca / BATCH) << "%\n";
}

// forward sobre todo el test, devuelve predicciones
std::vector<float> evaluarModelo(const std::vector<float>& testImages,
    float* gpuWeightsH, float* gpuBiasH, float* gpuWeightsO, float* gpuBiasO) {

    float* gpuTestImages, * gpuTestHidden, * gpuTestPredictions;
    cudaMalloc(&gpuTestImages, TEST_TOTAL * PIXELS * sizeof(float));
    cudaMalloc(&gpuTestHidden, TEST_TOTAL * HIDDEN * sizeof(float));
    cudaMalloc(&gpuTestPredictions, TEST_TOTAL * CLASSES * sizeof(float));
    cudaMemcpy(gpuTestImages, testImages.data(), testImages.size() * sizeof(float), cudaMemcpyHostToDevice);

    dim3 test_thr(256);
    dim3 test_blk_h((TEST_TOTAL + 255) / 256, HIDDEN);
    dim3 test_blk_o((TEST_TOTAL + 255) / 256, CLASSES);

    forwardHidden << <test_blk_h, test_thr >> > (gpuTestImages, gpuWeightsH, gpuBiasH, gpuTestHidden, TEST_TOTAL, PIXELS, HIDDEN);
    forwardOutput << <test_blk_o, test_thr >> > (gpuTestHidden, gpuWeightsO, gpuBiasO, gpuTestPredictions, TEST_TOTAL, HIDDEN, CLASSES);
    cudaDeviceSynchronize();

    std::vector<float> testPredictions(TEST_TOTAL * CLASSES);
    cudaMemcpy(testPredictions.data(), gpuTestPredictions, testPredictions.size() * sizeof(float), cudaMemcpyDeviceToHost);

    cudaFree(gpuTestImages);
    cudaFree(gpuTestHidden);
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

    std::vector<std::vector<int>> byDigit(CLASSES);
    for (int i = 0; i < TRAIN_TOTAL; ++i)
        byDigit[trainLabels[i]].push_back(i);

    float limite_h = sqrt(6.0f / (PIXELS + HIDDEN));
    float limite_o = sqrt(6.0f / (HIDDEN + CLASSES));
    std::vector<float> w_h(HIDDEN * PIXELS), w_o(CLASSES * HIDDEN);
    for (float& v : w_h) v = randFloat(-limite_h, limite_h);
    for (float& v : w_o) v = randFloat(-limite_o, limite_o);

    // init gpu
    float* gpuImages, * gpuHidden, * gpuPredictions, * gpuExpectedOutput, * gpuGradHidden;
    float* gpuWeightsH, * gpuBiasH, * gpuWeightsO, * gpuBiasO;
    inicializarGPU(gpuImages, gpuHidden, gpuPredictions, gpuExpectedOutput, gpuGradHidden,
        gpuWeightsH, gpuBiasH, gpuWeightsO, gpuBiasO, w_h, w_o);

    // entrenamiento
    std::vector<float> batchImages(BATCH * PIXELS);
    std::vector<float> batchExpectedOutput(BATCH * CLASSES, 0.0f);

    std::cout << "entrenando mlp con relu...\n";
    for (int epoch = 0; epoch < EPOCAS; ++epoch) {
        ejecutarEpoca(trainImages, byDigit, batchImages, batchExpectedOutput,
            gpuImages, gpuHidden, gpuPredictions, gpuExpectedOutput, gpuGradHidden,
            gpuWeightsH, gpuBiasH, gpuWeightsO, gpuBiasO, epoch + 1);
    }

    // evaluacion
    std::cout << "\nevaluando con " << TEST_TOTAL << " imagenes...\n";
    auto testPredictions = evaluarModelo(testImages, gpuWeightsH, gpuBiasH, gpuWeightsO, gpuBiasO);
    mostrarResultados(testPredictions, testLabels);

    cudaFree(gpuImages); cudaFree(gpuHidden); cudaFree(gpuPredictions);
    cudaFree(gpuExpectedOutput); cudaFree(gpuGradHidden);
    cudaFree(gpuWeightsH); cudaFree(gpuBiasH); cudaFree(gpuWeightsO); cudaFree(gpuBiasO);

    return 0;
}
