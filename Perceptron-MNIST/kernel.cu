#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include "kernel.h"

#define PIXELS_IMAGE 784

// cada hilo maneja una imagen y una neurona
__global__ void forward( const float* images,const float* weights, float* predictions, int totalImages, int totalClasses, int useThreshold) {

    int imageIndex = blockIdx.x * blockDim.x + threadIdx.x;
    int neuronIndex = blockIdx.y;

    if (imageIndex < totalImages && neuronIndex < totalClasses) {

        float weightedSum = weights[neuronIndex * 785 + 0]; // bias
        for (int i = 1; i <= PIXELS_IMAGE; ++i) 
            weightedSum += images[imageIndex * PIXELS_IMAGE + (i - 1)] * weights[neuronIndex * 785 + i];

        int outIndex = imageIndex * totalClasses + neuronIndex;


        if (useThreshold == 1) { // entrenamiento: 0 o 1
            if (weightedSum >= 0.0f)  predictions[outIndex] = 1.0f;
            else predictions[outIndex] = 0.0f;
        }
        else // evaluacion, valor real para argmax
            predictions[outIndex] = weightedSum;
    }
}

// si hubo error ajusta bias y pesos con la regla del perceptron
__global__ void backward( const float* images, const float* expectedOutput, const float* predictions, float* weights, float learningRate, int totalImages, int totalClasses) {

    int imageIndex = blockIdx.x * blockDim.x + threadIdx.x;
    int neuronIndex = blockIdx.y;

    if (imageIndex < totalImages && neuronIndex < totalClasses) {

        int   outIndex = imageIndex * totalClasses + neuronIndex;
        float error = expectedOutput[outIndex] - predictions[outIndex];

        if (error != 0.0f) {
            atomicAdd(&weights[neuronIndex * 785 + 0], learningRate * error); // ajusta bias
            for (int i = 1; i <= PIXELS_IMAGE; ++i) {
                float px = images[imageIndex * PIXELS_IMAGE + (i - 1)];
                atomicAdd(&weights[neuronIndex * 785 + i], learningRate * error * px);
            }
        }
    }
}