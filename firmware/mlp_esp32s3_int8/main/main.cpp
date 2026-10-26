#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <limits>
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bomba_int8.h"
#include "acumulador_int8.h"
#include "amostras_esp32.h"
#include "esp_timer.h"

// Estrutura dos modelos quantizados
struct ModeloINT8 {
    int entradas;
    int ocultos;
    int saidas;

    const int32_t* indices;
    const int32_t* classes;

    const float* media;
    const float* escala;

    const int8_t* W1;
    const float* escala_W1;
    const float* b1;

    const int8_t* W2;
    const float* escala_W2;
    const float* b2;

    float escala_entrada;
    float escala_oculta;
};

// Modelo da bomba
static const ModeloINT8 modelo_bomba = {
    BOMBA_ENTRADAS,
    BOMBA_OCULTOS,
    BOMBA_SAIDAS,

    bomba_indices,
    bomba_classes,

    bomba_media,
    bomba_escala,

    bomba_W1_int8,
    bomba_escala_W1,
    bomba_b1,

    bomba_W2_int8,
    bomba_escala_W2,
    bomba_b2,

    bomba_escala_entrada,
    bomba_escala_oculta
};

// Modelo do acumulador
static const ModeloINT8 modelo_acumulador = {
    ACUMULADOR_ENTRADAS,
    ACUMULADOR_OCULTOS,
    ACUMULADOR_SAIDAS,

    acumulador_indices,
    acumulador_classes,

    acumulador_media,
    acumulador_escala,

    acumulador_W1_int8,
    acumulador_escala_W1,
    acumulador_b1,

    acumulador_W2_int8,
    acumulador_escala_W2,
    acumulador_b2,

    acumulador_escala_entrada,
    acumulador_escala_oculta
};

// Quantização simétrica INT8
int8_t quantizar(float valor, float escala) {

    float q = std::nearbyint(valor / escala);

    if (q < -127.0f) q = -127.0f;
    if (q > 127.0f) q = 127.0f;

    return static_cast<int8_t>(q);
}

// Inferência quantizada
int32_t prever_int8(const float* X, const ModeloINT8& m) {

    int8_t entrada[ACUMULADOR_ENTRADAS];
    int8_t oculta[ACUMULADOR_OCULTOS];

    // Seleção, padronização e quantização
    for (int i = 0; i < m.entradas; i++) {

        float valor = (
            X[m.indices[i]] - m.media[i]
        ) / m.escala[i];

        entrada[i] = quantizar(
            valor,
            m.escala_entrada
        );
    }

    // Primeira camada: INT8 x INT8 -> INT32
    for (int j = 0; j < m.ocultos; j++) {

        int32_t acumulador = 0;

        for (int i = 0; i < m.entradas; i++) {

            acumulador +=
                int32_t(entrada[i]) *
                int32_t(m.W1[i * m.ocultos + j]);
        }

        // Reescala e viés
        float valor =
            float(acumulador) *
            (m.escala_entrada * m.escala_W1[j])
            + m.b1[j];

        // ReLU
        if (valor < 0.0f) {
            valor = 0.0f;
        }

        // Quantização da camada oculta
        oculta[j] = quantizar(
            valor,
            m.escala_oculta
        );
    }

    // Segunda camada: INT8 x INT8 -> INT32
    int melhor = 0;

    float maior =
        -std::numeric_limits<float>::infinity();

    for (int k = 0; k < m.saidas; k++) {

        int32_t acumulador = 0;

        for (int j = 0; j < m.ocultos; j++) {

            acumulador +=
                int32_t(oculta[j]) *
                int32_t(m.W2[j * m.saidas + k]);
        }

        float valor =
            float(acumulador) *
            (m.escala_oculta * m.escala_W2[k])
            + m.b2[k];

        if (valor > maior) {
            maior = valor;
            melhor = k;
        }
    }

    return m.classes[melhor];
}

// Verificar as previsões com os resultados do PC
template <size_t N, size_t F>
void verificar(
    const char* nome,
    const float (&amostras)[N][F],
    const int32_t (&esperados)[N],
    const ModeloINT8& modelo
) {

    int coincidencias = 0;

    printf("\n%s — INT8\n", nome);

    for (size_t i = 0; i < N; i++) {

        int32_t resultado = prever_int8(
            amostras[i],
            modelo
        );

        bool igual = resultado == esperados[i];

        if (igual) {
            coincidencias++;
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
        coincidencias,
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

// Ponto de entrada ESP-IDF
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

    printf("\n=== LATENCIA INT8 — QEMU ===\n");

    medir_latencia(
        "Bomba",
        bomba_amostras,
        modelo_bomba,
        prever_int8
    );

    medir_latencia(
        "Acumulador",
        acumulador_amostras,
        modelo_acumulador,
        prever_int8
    );

    // Medir depois das inferências
    medir_memoria("DEPOIS");

    printf("\nVerificacao FP32 concluida.\n");
}