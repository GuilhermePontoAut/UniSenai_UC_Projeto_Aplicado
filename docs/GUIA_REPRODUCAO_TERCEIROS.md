# Guia de reprodução para pesquisadores externos

> **Estado (26/10/2026):** os experimentos Python foram reexecutados no Ubuntu do autor; os firmwares FP32 e INT8 foram compilados e executados no QEMU nesse mesmo computador. **O procedimento completo abaixo ainda não foi validado de ponta a ponta por uma terceira pessoa em instalação limpa.** Registre dificuldades e diferenças, sem alterar silenciosamente o código ou os resultados históricos.

Este roteiro reproduz dois procedimentos distintos: **(A) treinamento e avaliação Python** a partir do dataset UCI e **(B) compilação e verificação funcional dos firmwares ESP32-S3** emulados com ESP-IDF/QEMU. A segunda parte utiliza parâmetros já versionados nos cabeçalhos C++ dos firmwares; executar o QEMU não treina novamente as redes.

## 0. Plataforma e requisitos

Ambiente de referência: **Ubuntu 22.04.5 LTS**, Python **3.10.12**, **ESP-IDF v6.1** e **QEMU Xtensa 9.2.2** (build `esp_develop_9.2.2_20260417`). Em outras distribuições, versões de Ubuntu, Windows ou macOS, o processo completo **não foi verificado**. Para código/versões detalhadas, consulte [AMBIENTE_PYTHON_ORIGINAL.md](AMBIENTE_PYTHON_ORIGINAL.md) e [REPRODUCAO_QEMU.md](REPRODUCAO_QEMU.md).

Instale o **Visual Studio Code**, com extensões **Python** e **Jupyter**, para a parte A. É possível usar outro editor compatível com `.ipynb`, mas esse fluxo não foi validado.

Abra um **terminal Ubuntu** para os comandos de shell; **não** digite comandos `git`, `code`, `apt` ou `idf.py` em células Python do notebook.

## A. Reprodução do notebook Python

### 1. Instalar bibliotecas do Ubuntu

No terminal Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y git unzip build-essential python3 \
  python3-numpy python3-pandas python3-sklearn \
  python3-scipy python3-matplotlib python3-ipykernel
```

O pacote `python3-ipykernel` está disponível no repositório **universe** do Ubuntu 22.04. Se o APT não o localizar, verifique se esse repositório está habilitado. A versão desse pacote no APT pode diferir de `ipykernel 6.29.5` observado no computador original. Para execução no VS Code não é necessário o pacote servidor `notebook`.

O notebook original utilizou bibliotecas científicas da distribuição Ubuntu, **não um venv instalado via `pip`**. Por isso, `pip install -r requirements.txt` não deve ser interpretado como reconstrução exata do ambiente da pesquisa.

Confira as versões:

```bash
python3 --version
python3 -c "import numpy,pandas,sklearn,scipy,matplotlib; print('NumPy',numpy.__version__); print('Pandas',pandas.__version__); print('Scikit-learn',sklearn.__version__); print('SciPy',scipy.__version__); print('Matplotlib',matplotlib.__version__)"
```

Versões originais: NumPy **1.21.5**, Pandas **1.3.5**, Scikit-learn **0.23.2**, SciPy **1.8.0**, Matplotlib **3.5.1**. As versões disponíveis via APT no novo computador podem diferir. **Registre essas diferenças em vez de presumir reprodução estrita.**

### 2. Obter o repositório

```bash
cd ~
git clone https://github.com/GuilhermePontoAut/UniSenai_UC_Projeto_Aplicado.git
cd UniSenai_UC_Projeto_Aplicado
git rev-parse --short HEAD
```

Guarde o hash mostrado: ele identifica a versão de código testada.

### 3. Baixar o dataset original

Baixe **Condition monitoring of hydraulic systems**, do [repositório oficial UCI](https://archive.ics.uci.edu/dataset/447/condition+monitoring+of+hydraulic+systems), usando o botão **Download** da página. O notebook não utiliza `ucimlrepo`: ele lê o ZIP original contendo, entre outros, `profile.txt`, `TS1.txt` e `PS1.txt`.

Copie o ZIP baixado para o local e nome **exatos** exigidos pelo código (substitua `/CAMINHO/DO/ARQUIVO_BAIXADO.zip` pelo caminho verdadeiro):

```bash
cp "/CAMINHO/DO/ARQUIVO_BAIXADO.zip" "notebooks/condition monitoring of hydraulic systems.zip"
unzip -t "notebooks/condition monitoring of hydraulic systems.zip" | tail -1
```

O teste de integridade deve terminar sem erros. **Não suba o ZIP nem os arquivos de sinais ao GitHub**; são dados obtidos de fonte pública separadamente.

### 4. Reexecutar sem modificar o notebook original

```bash
cd notebooks
cp Algoritmo.ipynb Algoritmo_reproducao.ipynb
code Algoritmo_reproducao.ipynb
```

Se o comando `code` não estiver no PATH, abra o VS Code pela interface e escolha `notebooks/Algoritmo_reproducao.ipynb`.

No VS Code:

1. Escolha o kernel Python do sistema (`/usr/bin/python3` em Ubuntu 22.04).
2. Na **cópia**, use **Restart / Restart Kernel** e depois **Run All**.
3. Espere a execução integral e confira se todas as células com código foram executadas, sem saídas de erro.
4. Não confunda **saídas antigas salvas no notebook original** com resultados recalculados.
5. Se houver `ConvergenceWarning` de Scikit-learn com `max_iter=1200`, **registre o aviso**, mas ele não é, por si só, uma exceção que interrompe a execução.
6. Guarde a cópia reexecutada e os valores medidos; não substitua o arquivo de referência do GitHub.

O notebook extrai o ZIP e grava artefatos em `notebooks/exportacao_esp32/`, inclusive cabeçalhos/NPZ. Esses arquivos são **resultados novos da execução Python**, distintos dos cabeçalhos históricos em `firmware/`.

### 5. Resultados preditivos a conferir

| Tarefa | Amostras de teste | F1 macro FP32 | F1 macro INT8 |
| --- | ---: | ---: | ---: |
| Bomba | 508 | 100,00% | 100,00% |
| Acumulador | 439 | 90,12% | 90,98% |

Outras referências: **GroupKFold do acumulador 68,82%** (média de 5 dobras); **0 divergências** nas previsões de C++ FP32 versus Scikit-learn nos testes reservados; **13 divergências** entre versões FP32 e INT8 no teste reservado do acumulador.

Esses números são referências do experimento publicado, **não resultados garantidos** em sistemas e versões diferentes. Registre valores divergentes e versões de dependências. Métricas de **tempo de execução** variam e não exigem coincidência exata.

## B. Reprodução da inferência C++ e do ESP32-S3/QEMU

### 6. Verificação auxiliar de C++ no próprio PC

Na raiz do repositório:

```bash
cd ~/UniSenai_UC_Projeto_Aplicado
python3 scripts/verificar_cpp_host.py
```

**Se o clone estiver em outro local, entre na pasta real do repositório**, em vez de usar o exemplo `~/UniSenai_UC_Projeto_Aplicado`.

Esperado para **cada** projeto (FP32 e INT8): **6/6** classificações coincidentes da bomba e **8/8** do acumulador. Esse teste usa cabeçalhos simulados das interfaces ESP-IDF e **não equivale a executar o microcontrolador emulado**.

### 7. Instalar ESP-IDF v6.1 e QEMU Xtensa

No terminal Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y git wget flex bison gperf python3 \
  python3-venv cmake ninja-build ccache libffi-dev libssl-dev \
  dfu-util libusb-1.0-0 libgcrypt20 libglib2.0-0 \
  libpixman-1-0 libsdl2-2.0-0 libslirp0

mkdir -p ~/esp
cd ~/esp
git clone --branch v6.1 --recursive https://github.com/espressif/esp-idf.git esp-idf-v6.1
cd esp-idf-v6.1
./install.sh esp32s3
python3 tools/idf_tools.py install qemu-xtensa
. ./export.sh

idf.py --version
qemu-system-xtensa --version
```

O método segue o [guia oficial do QEMU para ESP32-S3 na documentação ESP-IDF v6.1](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32s3/api-guides/tools/qemu.html). O QEMU precisa ser o **fork da Espressif**, não uma instalação genérica incompatível. O ambiente original da pesquisa usou QEMU 9.2.2, porém o instalador pode fornecer outro build; registre a versão efetiva.

**Atenção:** o comando `. ./export.sh` (ou `source .../export.sh`) deve ser repetido em cada novo terminal. Não é necessário executar essas etapas no terminal integrado do VS Code: o terminal Ubuntu é suficiente.

### 8. Compilar e executar os dois firmwares

Volte à raiz do clone do projeto, no **mesmo terminal em que o ESP-IDF está ativado**, e execute:

```bash
cd ~/UniSenai_UC_Projeto_Aplicado
python3 scripts/reproduzir_qemu.py --projeto ambos
```

Se o clone estiver em outro diretório, adapte o `cd` acima. O script executa `idf.py build` para cada firmware e executa no QEMU as verificações funcionais.

Resultado esperado ao concluir as duas execuções:

```text
OK: fp32 passou (6/6 bomba, 8/8 acumulador).
OK: int8 passou (6/6 bomba, 8/8 acumulador).
```

Confira as quatro contagens nos logs:

```bash
grep -H 'Resultado:' artefatos_reproducao/qemu/fp32.log artefatos_reproducao/qemu/int8.log
```

Se `idf.py` ou `qemu-system-xtensa` não forem encontrados, reative o ESP-IDF:

```bash
. ~/esp/esp-idf-v6.1/export.sh
```

Se aparecer erro `libslirp`, confira a instalação do pacote `libslirp0`. Consulte [REPRODUCAO_QEMU.md](REPRODUCAO_QEMU.md) para detalhes.

### 9. Interpretar corretamente o que foi reproduzido

- O **teste completo de F1** usa 508 ciclos da bomba e 439 do acumulador e é executado **no PC**, não no QEMU.
- Cada firmware no QEMU testa **14 exemplos selecionados do desenvolvimento** (6 bomba + 8 acumulador), verificando equivalência funcional, **não F1 do conjunto reservado**.
- As versões C++ INT8 usam **precisão mista**; não são redes integer-only.
- As configurações de build dos firmwares históricos diferem (`MINIMAL_BUILD` no FP32, sem a mesma opção no INT8). Não atribua diferenças do tamanho total dos firmwares ou de memória livre somente à quantização.
- O firmware INT8 contém mensagens de console herdadas com texto “FP32”; siga as contagens e a identificação do projeto, não apenas a mensagem.
- Não foram realizados testes em ESP32-S3 físico nem medição direta de energia. Valores de latência do QEMU são **exploratórios**.

## C. Envio das evidências ao autor

Envie um relatório breve com:

1. Sistema operacional e arquitetura (`lsb_release -a`, `uname -m`).
2. Commit testado (`git rev-parse --short HEAD`).
3. Versões Python/NumPy/Pandas/Scikit-learn/SciPy/Matplotlib; se aplicável, `ipykernel`.
4. Notebook novo **executado integralmente**, avisos, erros e os resultados finais de F1.
5. Resultados do script `verificar_cpp_host.py`.
6. Versões `idf.py --version` e `qemu-system-xtensa --version`.
7. Resultado completo do script QEMU e arquivos `artefatos_reproducao/qemu/fp32.log` e `int8.log`.
8. Eventuais adaptações necessárias para a reprodução.

O repositório ignora ZIP da UCI, sinais extraídos, artefatos gerados e logs locais. **Não publique os dados brutos, logs com informações pessoais, nem credenciais sem revisar o conteúdo.**

## Estado de conclusão

O experimento só poderá ser considerado **reproduzido independentemente** depois de outro pesquisador executar os dois fluxos, relatar seu ambiente e confrontar os resultados. Uma falha ou divergência é um achado relevante para melhorar documentação, dependências e/ou código; **não substitua valores divergentes pelos resultados históricos**.

Documentos complementares: [REPRODUCAO.md](REPRODUCAO.md), [AMBIENTE_PYTHON_ORIGINAL.md](AMBIENTE_PYTHON_ORIGINAL.md), [REPRODUCAO_QEMU.md](REPRODUCAO_QEMU.md), [VALIDACAO_REEXECUCAO_2026-10-26.md](VALIDACAO_REEXECUCAO_2026-10-26.md).
