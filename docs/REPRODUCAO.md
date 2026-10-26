# Reproduzindo os experimentos em Python

## O que este repositório reproduz

O notebook contém as etapas experimentais desde o carregamento dos sinais até os testes reservados e a auditoria metodológica, incluindo geração de parâmetros FP32 e INT8 e compilação/verificação de código C++ no computador.

Os projetos originais ESP-IDF FP32 e INT8 estão publicados em `firmware/`. Para a execução no ESP32-S3 emulado, siga [REPRODUCAO_QEMU.md](REPRODUCAO_QEMU.md). A reprodução independente em ambiente limpo e o congelamento completo das dependências Python permanecem etapas a verificar.

## 1. Pré-requisitos

- Linux, Python 3 e ambiente de notebook Jupyter.
- Para aproximar o ambiente original, consulte primeiro [AMBIENTE_PYTHON_ORIGINAL.md](AMBIENTE_PYTHON_ORIGINAL.md): Ubuntu 22.04.5 LTS, pacotes científicos APT e kernel Jupyter do usuário.
- O `requirements.txt` lista dependências sem fixar versões; não é um arquivo de reprodução estrita.
- Compilador `g++` disponível para os testes nativos em C++.

**Atenção:** o kernel original utiliza Python 3.10.12 com NumPy 1.21.5, Pandas 1.3.5, Scikit-learn 0.23.2, SciPy 1.8.0 e Matplotlib 3.5.1, instalados como pacotes da distribuição Ubuntu. O `ipykernel` 6.29.5 e o `jupyter_client` 8.6.3 estão em `~/.local`; o pacote `notebook` não foi encontrado nesse interpretador. Consulte o inventário para versões dos pacotes APT e limitações.

## 2. Dados

Baixe o conjunto original da [UCI](https://archive.ics.uci.edu/dataset/447/condition+monitoring+of+hydraulic+systems), sem redistribuir os arquivos brutos no GitHub.

O notebook inicia com:

```python
DATA_DIR = Path(".")
ZIP_PATH = DATA_DIR / "condition monitoring of hydraulic systems.zip"
```

Portanto, preserve o nome `condition monitoring of hydraulic systems.zip` no **diretório de trabalho do kernel** (a pasta `notebooks/` ao seguir o comando abaixo). O código extrai os arquivos do ZIP e lê `profile.txt` e os arquivos dos sensores.

## 3. Ambiente e abertura

Para aproximar o ambiente original, use Ubuntu 22.04, bibliotecas científicas via APT e as versões registradas em [AMBIENTE_PYTHON_ORIGINAL.md](AMBIENTE_PYTHON_ORIGINAL.md). **Essa etapa ainda não foi validada em instalação limpa e o APT atual pode fornecer atualizações diferentes.**

Abra `notebooks/Algoritmo.ipynb` no VS Code com o interpretador correto (no ambiente original, `/usr/bin/python3` e `ipykernel` disponível em `~/.local`). Não é necessário instalar o servidor clássico `notebook` se o VS Code já consegue executar o kernel.

Como alternativa **não equivalente ao ambiente histórico**, um ambiente virtual com `pip install -r requirements.txt` pode ajudar a explorar o notebook, mas não deve ser usado para alegar reprodução exata. Copie o ZIP da UCI para `notebooks/` antes de executar as células em sequência; registre quaisquer diferenças de resultado.

## 4. Saídas e verificação

O notebook gera arquivos em `exportacao_esp32/` e utiliza `g++` para verificações de inferência em C++ no PC. Não confunda resultados salvos anteriormente nas células com evidências de uma execução nova no seu ambiente.

Ao interpretar o estudo, preserve as distinções entre:
- validação por grupos e teste reservado;
- FP32 e INT8 de precisão mista;
- arrays exportados, bibliotecas ligadas ao firmware e tamanho total do firmware;
- resultados no PC, resultados de execução emulada e medições futuras na placa real.

## 5. QEMU / ESP-IDF

A versão relatada utilizou ESP-IDF v6.1 e ESP32-S3 emulado. Os fontes originais FP32 e INT8 encontram-se em `firmware/`; consulte [REPRODUCAO_QEMU.md](REPRODUCAO_QEMU.md) para comandos, configuração e limitações. Os arquivos foram copiados diretamente dos projetos originais, sem reconstruí-los a partir do notebook.

## 6. Ética de reprodução e limites

A separação treino/teste é fixa para o experimento registrado, e a validação agrupada apresentou forte variabilidade na classificação do acumulador. O desempenho na partição reservada não é garantia de extrapolação para equipamentos ou condições de falha desconhecidos.
