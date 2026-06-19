#ifndef KERNEL_H
#define KERNEL_H

// capa oculta (relu)
__global__ void forwardHidden(
    const float* images, 
    const float* w_h, 
    const float* b_h,
    float* hidden_out, 
    int batchSize, 
    int pixels, 
    int hiddenSize);

// capa de salida (lineal)
__global__ void forwardOutput(
    const float* hidden_out, 
    const float* w_o, 
    const float* b_o,
    float* predictions, 
    int batchSize, 
    int hiddenSize, 
    int classes);

// backward para capa de salida (lineal)
__global__ void backwardOutput(
    const float* hidden_out, 
    const float* expected, 
    const float* predictions,
    float* w_o, 
    float* b_o, 
    float* grad_hidden,
    int batchSize, 
    int hiddenSize, 
    int classes, 
    float lr);

// backward para capa oculta (relu)
__global__ void backwardHidden(
    const float* images, 
    const float* hidden_out, 
    const float* grad_hidden,
    float* w_h, 
    float* b_h, 
    int batchSize, 
    int pixels, 
    int hiddenSize, 
    float lr);

#endif
