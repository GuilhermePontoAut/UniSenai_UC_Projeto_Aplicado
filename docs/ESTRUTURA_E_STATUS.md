# Estado da publicação

## Incluído agora

- `notebooks/Algoritmo.ipynb`: cópia integral, sem alterar células nem resultados salvos, da última versão fornecida (`Algoritmo(10).ipynb`).
- `README.md`, `requirements.txt` e `.gitignore`.
- Documentação do dataset e do processo experimental.

## Projetos embarcados publicados e validação pendente

- Projeto ESP-IDF FP32 original incluído em `firmware/mlp_esp32s3/`.
- Projeto ESP-IDF INT8 original incluído em `firmware/mlp_esp32s3_int8/`.
- Parâmetros e amostras de verificação necessários já estão nos cabeçalhos C++ versionados.
- `docs/REPRODUCAO_QEMU.md` documenta opções, comandos, diferenças de configuração e limites. Falta validar o processo inteiro em ambiente limpo.

**Importante:** não incluir `build/`, arquivos pessoais, dados do dataset nem credenciais no repositório.

Os códigos originais já estão publicados, porém a execução integral ainda deve ser repetida independentemente em um ambiente limpo com ESP-IDF e QEMU.

## Histórico do notebook

Esta publicação prioriza `Algoritmo(10).ipynb`, que acrescenta auditorias metodológicas à versão 9. A cópia foi mantida com os outputs originais e deve ser considerada registro da execução anterior, não uma nova execução.
