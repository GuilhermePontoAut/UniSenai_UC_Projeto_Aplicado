# Reprodução dos firmwares ESP32-S3 / QEMU

Este diretório documenta como compilar e executar **os dois projetos ESP-IDF originalmente utilizados** no estudo de MLPs compactas para diagnóstico hidráulico. Os fontes foram disponibilizados pelo autor; nenhuma linha de `main.cpp`, cabeçalho, `sdkconfig` ou arquivo CMake foi alterada nesta publicação.

## 1. Escopo e limites das evidências

- `firmware/mlp_esp32s3/`: firmware de inferência **FP32** para as duas MLPs.
- `firmware/mlp_esp32s3_int8/`: firmware de inferência **INT8 de precisão mista** para as duas MLPs.
- Cada firmware verifica **6 ciclos da bomba e 8 do acumulador**, incorporados em `main/amostras_esp32.h` com resultados esperados. Esses ciclos são exemplos de desenvolvimento, não o conjunto de teste reservado.
- O teste reservado completo — **508 ciclos da bomba + 439 do acumulador** — foi avaliado **no PC**, conforme o notebook, e não no QEMU.
- As rotinas de inferência são implementações próprias em C++, sem TensorFlow Lite ou ESP-NN. Não há aquisição de sinais nem extração de características em tempo real no firmware.
- Emulação não equivale a validação em ESP32-S3 físico. Tempos medidos no QEMU não estimam diretamente a latência real da placa.

## 2. Ambiente utilizado e recomendado

- Ubuntu/Linux, `git`, CMake/Ninja e ferramentas usuais de compilação.
- **ESP-IDF v6.1** para alvo `esp32s3` (ambos `sdkconfig` registram `CONFIG_IDF_INIT_VERSION="6.1.0"`).
- **Fork da Espressif do QEMU Xtensa**, com suporte a ESP32-S3, instalado via `idf_tools.py`.
- Linux: biblioteca dinâmica `libslirp0` e demais dependências do QEMU.
- **Versões verificadas na reexecução de 26/10/2026 (Ubuntu do pesquisador):** `ESP-IDF v6.1`; `QEMU emulator version 9.2.2 (esp_develop_9.2.2_20260417)`. A revisão exata do ESP-IDF além de `v6.1` não foi registrada. Em outras instalações, registre `idf.py --version` e `qemu-system-xtensa --version` para interpretar eventuais diferenças.

Exemplo de instalação em Ubuntu/Debian. Adapte caso o ESP-IDF já esteja instalado:

```bash
sudo apt-get update
sudo apt-get install -y git wget flex bison gperf python3 python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0 \
  libgcrypt20 libglib2.0-0 libpixman-1-0 libsdl2-2.0-0 libslirp0
mkdir -p ~/esp
cd ~/esp
git clone --branch v6.1 --recursive https://github.com/espressif/esp-idf.git esp-idf-v6.1
cd esp-idf-v6.1
./install.sh esp32s3
python tools/idf_tools.py install qemu-xtensa
. ./export.sh
idf.py --version
qemu-system-xtensa --version
```

Instalação e uso do QEMU são descritos pela Espressif: https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32s3/api-guides/tools/qemu.html

**Importante:** a inicialização do ESP-IDF (`. /caminho/esp-idf/export.sh`) deve ser realizada em cada novo shell antes de executar `idf.py`.

## 3. Reproduzir as verificações no QEMU

A partir da **raiz do repositório**, após ativar o ESP-IDF:

```bash
python3 scripts/reproduzir_qemu.py --projeto fp32
python3 scripts/reproduzir_qemu.py --projeto int8
```

O script compila cada projeto (`idf.py build`), executa o QEMU, coleta seu log e verifica as duas contagens `Resultado: 6/6` e `Resultado: 8/8`. Por padrão, salva logs em `artefatos_reproducao/qemu/`, diretório que não deve ser versionado. Pode usar `--projeto ambos` para verificar os dois.

**Execução manual (alternativa)**:

```bash
cd firmware/mlp_esp32s3
idf.py build
idf.py qemu monitor
```

Depois repita em `firmware/mlp_esp32s3_int8`. Saia do monitor do ESP-IDF usando a sequência indicada pelo próprio monitor (geralmente `Ctrl+]`).

**Sem QEMU:** rode `python3 scripts/verificar_cpp_host.py` para uma conferência auxiliar, que compila os mesmos fontes com interfaces ESP-IDF simuladas. **Este teste não substitui a compilação ESP-IDF nem a execução no QEMU.**

## 4. Resultados esperados e indicadores

Para **cada firmware**, são esperadas as duas contagens:

```text
BOMBA:      6/6 classificacoes coincidentes
ACUMULADOR: 8/8 classificacoes coincidentes
```

Os projetos também imprimem leituras de heap, margem de pilha e médias de latência de inferência. Esses números dependem do ambiente, configuração e carga da emulação. Os resultados históricos relatados no artigo não constituem tolerâncias de aprovação automatizada.

| Indicador histórico no QEMU | FP32 | INT8 |
|---|---:|---:|
| Imagem de aplicação (bytes) | 161.365 | 177.732 |
| `libmain.a` (bytes) | 11.266 | 7.382 |
| Heap livre final (bytes) | 400.308 | 396.956 |
| Pilha livre mínima (bytes) | 2.976 | 2.808 |
| Bomba (µs/inferência) | 4,5919 | 14,5907 |
| Acumulador (µs/inferência) | 12,9332 | 36,0469 |

## 5. Alertas encontrados na auditoria dos arquivos originais

1. **Configuração de build diferente.** O `CMakeLists.txt` FP32 define `idf_build_set_property(MINIMAL_BUILD ON)`, enquanto o INT8 **não** define `MINIMAL_BUILD`. Os arquivos `sdkconfig` também têm conjuntos de opções diferentes. Logo, comparações diretas do tamanho de firmware ou da RAM livre incluem fatores além da quantização. **Não atribua a diferença dessas medidas somente à representação INT8.**
2. **Mensagens desatualizadas no INT8.** Em `firmware/mlp_esp32s3_int8/main/main.cpp`, a identificação inicial e a mensagem final mencionam `FP32`, embora o código utilize `prever_int8`. A amostragem consulta `*_esperado_fp32`, e os valores são idênticos aos correspondentes `*_esperado_int8` para as 14 amostras do cabeçalho atual. O código foi preservado literalmente para manter a proveniência e a possibilidade de repetir os resultados históricos.
3. **Projeto FP32 herdou o nome `hello_world`.** O `CMakeLists.txt` raiz usa `project(hello_world)`, apesar de o `main.cpp` compilar a MLP. Esse nome não altera a lógica da inferência.
4. **Templates omitidos.** O arquivo compactado incluía `hello_world_main.c`, `pytest_hello_world.py`, um README padrão do exemplo da Espressif, `sdkconfig.ci` vazio e um `sdkconfig.backup` idêntico ao `sdkconfig` INT8. Esses arquivos não são compilados nas configurações dos projetos ou eram redundantes; foram excluídos da distribuição mínima. Os 16 arquivos relevantes publicados correspondem byte a byte aos originais, conforme `firmware/SHA256_ORIGINAIS.json`.
5. **Verificação realizada durante a preparação desta publicação.** Ambos os fontes compilaram com `g++` e interfaces ESP-IDF simuladas no host e produziram 6/6 e 8/8 classificações coincidentes cada um. **Não houve uma nova compilação ESP-IDF nem nova execução QEMU neste ambiente de preparação**, pois essas ferramentas não estavam disponíveis. **Posteriormente, o pesquisador compilou e executou ambos os firmwares no QEMU em seu Ubuntu**, confirmando as contagens 6/6 e 8/8; veja [o registro da reexecução](VALIDACAO_REEXECUCAO_2026-10-26.md). Isso ainda não substitui o teste em um computador independente.

## 6. Reproduzir o experimento completo desde os dados

1. Baixe o dataset *Condition Monitoring of Hydraulic Systems* diretamente do repositório UCI (https://archive.ics.uci.edu/dataset/447/condition+monitoring+of+hydraulic+systems).
2. Use `docs/REPRODUCAO.md` para preparar o ambiente Python e executar `notebooks/Algoritmo.ipynb` **na ordem**.
3. Confira os conjuntos de desenvolvimento/teste, seleção de características, redes finais, exportação dos parâmetros, verificação em C++ no PC e resultados de teste reservados.
4. Use os cabeçalhos versionados em `firmware/` como **snapshots dos parâmetros efetivamente utilizados nos experimentos QEMU originais**. Para investigar regeneração bit a bit a partir do notebook, compare os cabeçalhos exportados com os snapshots antes de substituí-los.
5. Execute ambos os firmwares no QEMU com os comandos da seção 3.

**Limitação de reprodutibilidade:** o `requirements.txt` original do repositório não congela versões exatas de todas as bibliotecas Python. Isso significa que uma nova instalação pode produzir resultados diferentes, mesmo seguindo os passos. A reprodução integral do treinamento em um ambiente limpo ainda exige congelamento e validação das versões históricas dos pacotes. Os snapshots C++ permitem reproduzir as inferências embarcadas independentemente do novo treinamento.

## 7. Recomendações para avaliação comparativa futura

- Preparar configurações de compilação equivalentes para FP32 e INT8, sem alterar a pasta histórica original.
- Medir no hardware físico para comparar latência, RAM e consumo de energia.
- Separar medidas de código, dados estáticos, pilha, heap e firmware completo.
- Acrescentar testes automatizados de equivalência entre parâmetros gerados no Python e snapshots C++.
