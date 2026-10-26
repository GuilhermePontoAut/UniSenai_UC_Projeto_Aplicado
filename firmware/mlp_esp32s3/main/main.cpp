#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <limits>
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bomba_fp32.h"
#include "acumulador_fp32.h"
#include "amostras_esp32.h"
#include "esp_timer.h"

// Estrutura genérica de uma MLP
struct ModeloMLP {
    int entradas;
    int ocultos;
    int saidas;

    const int32_t* indices;
    const int32_t* classes;

    const float* media;
    const float* escala;

    const float* W1;
    const float* b1;
    const float* W2;
    const float* b2;
};

// Modelo da bomba
static const ModeloMLP modelo_bomba = {
    BOMBA_ENTRADAS,
    BOMBA_OCULTOS,
    BOMBA_SAIDAS,
    bomba_indices,
    bomba_classes,
    bomba_media,
    bomba_escala,
    bomba_W1,
    bomba_b1,
    bomba_W2,
    bomba_b2
};

// Modelo do acumulador
static const ModeloMLP modelo_acumulador = {
    ACUMULADOR_ENTRADAS,
    ACUMULADOR_OCULTOS,
    ACUMULADOR_SAIDAS,
    acumulador_indices,
    acumulador_classes,
    acumulador_media,
    acumulador_escala,
    acumulador_W1,
    acumulador_b1,
    acumulador_W2,
    acumulador_b2
};

// Inferência FP32
int32_t prever_fp32(const float* X, const ModeloMLP& m) {

    float entrada[ACUMULADOR_ENTRADAS];
    float oculta[ACUMULADOR_OCULTOS];

    // Seleção e padronização
    for (int i = 0; i < m.entradas; i++) {
        entrada[i] = (
            X[m.indices[i]] - m.media[i]
        ) / m.escala[i];
    }

    // Primeira camada + ReLU
    for (int j = 0; j < m.ocultos; j++) {

        float soma = m.b1[j];

        for (int i = 0; i < m.entradas; i++) {
            soma += entrada[i] *
                m.W1[i * m.ocultos + j];
        }

        oculta[j] = soma > 0.0f ? soma : 0.0f;
    }

    // Camada de saída
    int melhor = 0;

    float maior =
        -std::numeric_limits<float>::infinity();

    for (int k = 0; k < m.saidas; k++) {

        float soma = m.b2[k];

        for (int j = 0; j < m.ocultos; j++) {
            soma += oculta[j] *
                m.W2[j * m.saidas + k];
        }

        if (soma > maior) {
            maior = soma;
            melhor = k;
        }
    }

    return m.classes[melhor];
}

// Comparação com as previsões do PC
template <size_t N, size_t F>
void verificar(
    const char* nome,
    const float (&amostras)[N][F],
    const int32_t (&esperados)[N],
    const ModeloMLP& modelo
) {

    int acertos = 0;

    printf("\n%s — FP32\n", nome);

    for (size_t i = 0; i < N; i++) {

        int32_t resultado = prever_fp32(
            amostras[i], modelo
        );

        bool igual = resultado == esperados[i];

        if (igual) {
            acertos++;
        }

        printf(
            "Amostra %u: esperado=%d, obtido=%d [%s]\n",
            (unsigned)(i + 1),
            (int)esperados[i],
            (int)resultado,
            igual ? "OK" : "ERRO"
        );
    }

    printf(
        "Resultado: %d/%u classificacoes coincidentes\n",
        acertos,
        (unsigned)N
    );
}

// Medição dos recursos de memória
void medir_memoria(const char* etapa) {

    printf("\n=== MEMORIA: %s ===\n", etapa);

    printf(
        "Heap livre: %u bytes\n",
        (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT)
    );

    printf(
        "Heap livre minimo: %u bytes\n",
        (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT)
    );

    printf(
        "Pilha livre minima: %u bytes\n",
        (unsigned)uxTaskGetStackHighWaterMark(NULL)
    );
}

// Benchmark da inferência
template <size_t N, size_t F, typename Modelo>
void medir_latencia(
    const char* nome,
    const float (&amostras)[N][F],
    const Modelo& modelo,
    int32_t (*prever)(const float*, const Modelo&)
) {
    const int repeticoes = 2000;
    volatile int64_t checksum = 0;

    // Aquecimento
    for (size_t i = 0; i < N; i++) {
        checksum += prever(amostras[i], modelo);
    }

    int64_t inicio = esp_timer_get_time();

    for (int r = 0; r < repeticoes; r++) {
        for (size_t i = 0; i < N; i++) {
            checksum += prever(amostras[i], modelo);
        }
    }

    int64_t fim = esp_timer_get_time();

    double latencia_us =
        double(fim - inicio) / (repeticoes * N);

    printf(
        "%s: %.4f us/inferencia (checksum=%lld)\n",
        nome,
        latencia_us,
        (long long)checksum
    );
}

// Ponto de entrada do ESP-IDF
extern "C" void app_main(void) {

    printf("\n=== EDGE AI — ESP32-S3 — FP32 ===\n");

    // Medir antes das inferências
    medir_memoria("ANTES");

    verificar(
        "BOMBA",
        bomba_amostras,
        bomba_esperado_fp32,
        modelo_bomba
    );

    verificar(
        "ACUMULADOR",
        acumulador_amostras,
        acumulador_esperado_fp32,
        modelo_acumulador
    );

    printf("\n=== LATENCIA FP32 — QEMU ===\n");

    medir_latencia(
        "Bomba",
        bomba_amostras,
        modelo_bomba,
        prever_fp32
    );

    medir_latencia(
        "Acumulador",
        acumulador_amostras,
        modelo_acumulador,
        prever_fp32
    );


    // Medir depois das inferências
    medir_memoria("DEPOIS");

    printf("\nVerificacao FP32 concluida.\n");    
}