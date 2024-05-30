#include <stdio.h>
#include <math.h>
// #include <stdlib.h>

#define T 20

// s(t)
double s(double t) {
    if (t >= 1 && t <= 5) 
        return 6;
    if (t > 5 && t <= 9) 
        return (19*1.5-t)*1.5;
    if (t > 9 && t <= 15) 
        return (23.5/1.5 - t) * 1.5;
    if (t > 15 && t <= 18) 
        return (t - 15 + (3.0/4.0)) * (4.0/3.0);
    if (t > 18 && t <= 20) 
        return 5;
    return 0;
}

// modulate impulse
void modulate(double phi, double *w, int size) {
    for (int i = 0; i < size; i++) {
        double t = i * (double)T / size;
        w[i] = s(t) * sin(2 * M_PI * phi * t);
    }
}

// signal sampling, fancy term - discretization
void discretize(double *w, double *wd, int size, double sm) {
    double p = (2 * M_PI) / sm;
    for (int i = 0; i < size; i++) {
        wd[i] = w[(int)(i * p)];
    }
}

// normalize signal
void normalize(double *wd, double *wn, int size, int levels) {
    double max = 0;
    for (int i = 0; i < size; i++) {
        if (fabs(wd[i]) > max) {
            max = fabs(wd[i]);
        }
    }
    for (int i = 0; i < size; i++) {
        wn[i] = (wd[i] / max) * (levels / 2.0);
    }
}

// quantize signal
void quantize(double *wn, int *q, int size, int levels) {
    for (int i = 0; i < size; i++) {
        q[i] = (int)(round(wn[i]));
        if (q[i] > levels / 2) 
            q[i] = levels / 2;
        if (q[i] < -levels / 2) 
            q[i] = -levels / 2;
    }
} // 2

// convert to binary
void to_binary(int *q, char (*binary)[16], int size, int levels) {
    int bits = log2(levels);
    for (int i = 0; i < size; i++) {
        int value = q[i] + levels / 2;
        for (int j = bits - 1; j >= 0; j--) {
            binary[i][j] = (value % 2) + '0'; // '1' or if not '0'
            value /= 2;
        }
        binary[i][bits] = '\0';
    }
}

int main() {
    const double phi = 0.59;
    const double sm = 33.64;
    const int levels8 = 8;
    const int levels16 = 16;
    const int size = 1000;

    // T - time step in out signal in ms
    // with 8 levels signal troughput will be
    // (1000 / 20) * 3 = 150 bps (bits per second)
    //
    // with 16 levels signal troughput will be
    // (1000 / 20) * 4 = 200 bps 

    double w[size], wd[size], wn[size];
    int q[size];
    char binary[size][16];

    // we create signal from task conditions
    modulate(phi, w, size);

    // we sample signal from analog stream
    discretize(w, wd, size, sm);

    // normalize for 8 levels of quantization
    normalize(wd, wn, size, levels8);
    quantize(wn, q, size, levels8);
    to_binary(q, binary, size, levels8);
 
    // output data
    printf("8 levels quantization binary code:\n\t");
    for (int i = 0; i < size; i++) {
        if (i % 10 < 1)
            printf("\n\t");
        printf("%s ", binary[i]);
    }

    // normalize for 16 levels of quantization
    normalize(wd, wn, size, levels16);
    quantize(wn, q, size, levels16);
    to_binary(q, binary, size, levels16);
 
    // output data
    printf("\n16 levels quantization binary code:\n");
    for (int i = 0; i < size; i++) {
        if (i % 10 < 1)
            printf("\n\t");
        printf("%s ", binary[i]);
    }

    return 0;
}
