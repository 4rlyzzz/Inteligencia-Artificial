#include <cuda_runtime.h>
#include <math.h>
#include "kernel.h"

// funciones auxiliares device
__device__ float relu(float x) { 
    return x > 0 ? x : 0; 
}
__device__ float drelu(float x) { 
    return x > 0 ? 1.0f : 0.0f; 
}

// forward de la capa oculta (relu)
__global__ void forwardHidden(const float* images, const float* w_h, const float* b_h,
    float* hidden_out, int batchSize, int pixels, int hiddenSize) {
    int img = blockIdx.x * blockDim.x + threadIdx.x;   // indice de imagen en el batch
    int neu = blockIdx.y; // indice de neurona oculta
    if (img >= batchSize || neu >= hiddenSize) return;

    float suma = b_h[neu];
    for (int i = 0; i < pixels; ++i) {
        suma += images[img * pixels + i] * w_h[neu * pixels + i];
    }
    hidden_out[img * hiddenSize + neu] = relu(suma);
}

// forward de la capa de salida (lineal)
__global__ void forwardOutput(const float* hidden_out, const float* w_o, const float* b_o,
    float* predictions, int batchSize, int hiddenSize, int classes) {
    int img = blockIdx.x * blockDim.x + threadIdx.x;
    int neu = blockIdx.y;
    if (img >= batchSize || neu >= classes) return;

    float suma = b_o[neu];
    for (int i = 0; i < hiddenSize; ++i) {
        suma += hidden_out[img * hiddenSize + i] * w_o[neu * hiddenSize + i];
    }
    predictions[img * classes + neu] = suma; 
}

// backward para capa de salida (lineal)
__global__ void backwardOutput(const float* hidden_out, const float* expected, const float* predictions,
    float* w_o, float* b_o, float* grad_hidden,
    int batchSize, int hiddenSize, int classes, float lr) {
    int img = blockIdx.x * blockDim.x + threadIdx.x;
    int neu = blockIdx.y;
    if (img >= batchSize || neu >= classes) return;

    float y = predictions[img * classes + neu];
    float d = expected[img * classes + neu];
    float error = d - y;   // derivada de la salida lineal es 1

    if (error != 0.0f) {
        // propagar error a la capa oculta (acumular en grad_hidden)
        for (int i = 0; i < hiddenSize; ++i) {
            float w = w_o[neu * hiddenSize + i];   // leer peso actual
            atomicAdd(&grad_hidden[img * hiddenSize + i], error * w);
        }
        float lr_eff = lr / batchSize;
        atomicAdd(&b_o[neu], lr_eff * error);
        for (int i = 0; i < hiddenSize; ++i) {
            float h = hidden_out[img * hiddenSize + i];
            atomicAdd(&w_o[neu * hiddenSize + i], lr_eff * error * h);
        }
    }
}

// backward para capa oculta (relu)
__global__ void backwardHidden(const float* images, const float* hidden_out, const float* grad_hidden,
    float* w_h, float* b_h, int batchSize, int pixels, int hiddenSize, float lr) {
    int img = blockIdx.x * blockDim.x + threadIdx.x;
    int neu = blockIdx.y;
    if (img >= batchSize || neu >= hiddenSize) return;

    float h = hidden_out[img * hiddenSize + neu];
    float delta = drelu(h) * grad_hidden[img * hiddenSize + neu];
    if (delta != 0.0f) {
        // promediar sobre el batch
        float lr_eff = lr / batchSize;
        atomicAdd(&b_h[neu], lr_eff * delta);
        for (int i = 0; i < pixels; ++i) {
            float x = images[img * pixels + i];
            atomicAdd(&w_h[neu * pixels + i], lr_eff * delta * x);
        }
    }
}
