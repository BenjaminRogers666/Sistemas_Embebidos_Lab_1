#include <stdio.h>
#include <inttypes.h>
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_timer.h"
#include "esp_cpu.h"

#define VECTOR_SIZE 20
#define ITERATIONS 1000

// Atributos de memoria usando definiciones oficiales
DRAM_ATTR static int vector_dram[VECTOR_SIZE];
IRAM_ATTR static int vector_iram[VECTOR_SIZE];
RTC_DATA_ATTR static int vector_rtc[VECTOR_SIZE];

void init_vectors() {
    for (int i = 0; i < VECTOR_SIZE; i++) {
        vector_dram[i] = 1;
        vector_iram[i] = 1;
        vector_rtc[i] = 1;
    }
}

uint32_t get_cycle_count() {
    // Solución universal que funciona en todas las versiones
    uint32_t cycles;
    asm volatile ("rsr %0, ccount" : "=r" (cycles));
    return cycles;
}

void measure_memory(const char* name, int* vector, int num, uint32_t* total_cycles) {
    uint64_t start_us = esp_timer_get_time();
    uint32_t start_cycles = get_cycle_count();
    
    int results[VECTOR_SIZE];
    for (int i = 0; i < ITERATIONS; i++) {
        for (int j = 0; j < VECTOR_SIZE; j++) {
            results[j] = vector[j] * num;
        }
        asm volatile("" : : "r"(results) : "memory");
    }
    
    uint64_t end_us = esp_timer_get_time();
    uint32_t end_cycles = get_cycle_count();
    
    *total_cycles = end_cycles - start_cycles;
    uint32_t cycles_per_op = *total_cycles / (ITERATIONS * VECTOR_SIZE);
    
    printf("%-6s: %5"PRIu64" us | %5"PRIu32" ciclos | %3"PRIu32" ciclos/op\n",
           name, (end_us - start_us), *total_cycles, cycles_per_op);
}


void print_ascii_chart(const char* names[], const uint32_t cycles[], int count) {
    printf("\nGráfico de Rendimiento Relativo:\n");
    printf("----------------------------------------\n");
    
    uint32_t max_cycles = 0;
    for (int i = 0; i < count; i++) {
        if (cycles[i] > max_cycles) max_cycles = cycles[i];
    }
    
    for (int i = 0; i < count; i++) {
        int bar_length = (cycles[i] * 50) / max_cycles;
        printf("%-6s: ", names[i]);
        for (int j = 0; j < bar_length; j++) printf("█");
        printf(" %"PRIu32" ciclos\n", cycles[i]);
    }
    printf("----------------------------------------\n");
}

void app_main() {
    init_vectors();
    const int num = 5;
    
    printf("\nEjercicio 4 - Comparación de Memorias\n");
    printf("Iteraciones: %d | Tamaño vector: %d\n\n", ITERATIONS, VECTOR_SIZE);
    
    uint32_t cycles[5] = {0};
    const char* names[] = {"DRAM", "IRAM", "RTC", "PSRAM", "FLASH"};
    
    // Mediciones
    measure_memory(names[0], vector_dram, num, &cycles[0]);
    measure_memory(names[1], vector_iram, num, &cycles[1]);
    measure_memory(names[2], vector_rtc, num, &cycles[2]);
    
    // PSRAM dinámica
    int *vector_psram = heap_caps_malloc(VECTOR_SIZE * sizeof(int), MALLOC_CAP_SPIRAM);
    if (vector_psram) {
        for (int i = 0; i < VECTOR_SIZE; i++) vector_psram[i] = 1;
        measure_memory(names[3], vector_psram, num, &cycles[3]);
        heap_caps_free(vector_psram);
    } else {
        printf("PSRAM: No disponible\n");
        cycles[3] = 0;
    }
    
    // Memoria FLASH
    const int vector_flash[VECTOR_SIZE] = {[0 ... VECTOR_SIZE-1] = 1};
    measure_memory(names[4], (int*)vector_flash, num, &cycles[4]);
    
    // Gráfico ASCII (usando ciclos totales)
    print_ascii_chart(names, cycles, 5);
    
    printf("\nMedición completada\n");
    
    // Análisis automático (corregido format specifiers)
    printf("\nAnálisis:\n");
    printf("- IRAM es %"PRIu32"x más rápida que DRAM\n", cycles[0]/cycles[1]);
    printf("- RTC es %"PRIu32"x más lenta que DRAM\n", cycles[2]/cycles[0]);
    printf("- PSRAM muestra %srendimiento\n", 
           (cycles[3] > 0 && cycles[3] < cycles[0]*2) ? "buen " : "bajo ");
}