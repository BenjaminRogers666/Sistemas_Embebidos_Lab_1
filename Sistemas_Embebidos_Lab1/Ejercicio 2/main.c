#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_attr.h"

// Variables globales
int var_1 = 233;
int var_2 = 128;
int X = 1000; // Valor inicial de iteraciones (ajústalo según pruebas)

// Función para calcular CPI
void calculate_metrics(uint64_t start_cycles, uint64_t end_cycles, uint64_t start_time, uint64_t end_time) {
    uint64_t total_cycles = end_cycles - start_cycles;
    uint64_t total_time_us = end_time - start_time;
    float cpi = (float)total_cycles / (X * 5); // 5 operaciones por iteración

    printf("\n--- Métricas ---\n");
    printf("Ciclos totales: %llu\n", total_cycles);
    printf("Tiempo total (us): %llu\n", total_time_us);
    printf("CPI: %.2f\n", cpi);
}

void app_main() {
    uint64_t start_cycles, end_cycles;
    uint64_t start_time, end_time;

    printf("Iniciando pruebas con X = %d...\n", X);

    // Iniciar mediciones
    start_cycles = esp_cpu_get_cycle_count();
    start_time = esp_timer_get_time();

    // Bucle de operaciones
    for (int i = 0; i < X; i++) {
        volatile int result_0 = var_1 + var_2;
        volatile int result_1 = var_1 + 10;
        volatile int result_2 = var_1 % var_2;
        volatile int result_3 = var_1 * var_2;
        volatile int result_4 = var_1 / var_2;
    }

    // Finalizar mediciones
    end_cycles = esp_cpu_get_cycle_count();
    end_time = esp_timer_get_time();

    // Calcular métricas
    calculate_metrics(start_cycles, end_cycles, start_time, end_time);
}