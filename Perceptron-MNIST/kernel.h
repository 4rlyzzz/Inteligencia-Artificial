#ifndef KERNEL_H
#define KERNEL_H

#include <cuda_runtime.h>

// calcula la prediccion de cada imagen para cada neurona
__global__ void forward(
    const float* images,
    const float* weights,
    float* predictions,
    int totalImages,
    int totalClasses,    
    int useThreshold); // 1=devuelve 0 o 1 (entrenar), 0=suma directa (evaluar)


// ajusta los pesos segun el error cometido
__global__ void backward(
    const float* images,
    const float* expectedOutput, // lo que deberia haber salido
    const float* predictions,    // lo que salio
    float* weights,
    float learningRate,   // que tan rapido ajusta los pesos
    int totalImages,
    int totalClasses);

#endif