# Estado da publicação

## Incluído agora

- `notebooks/Algoritmo.ipynb`: cópia integral, sem alterar células nem resultados salvos, da última versão fornecida (`Algoritmo(10).ipynb`).
- `README.md`, `requirements.txt` e `.gitignore`.
- Documentação do dataset e do processo experimental.

## Ainda necessário para reprodução embarcada completa

- Projeto ESP-IDF FP32 original: `CMakeLists.txt`, `main/`, `sdkconfig` aplicável e outros fontes usados na compilação do QEMU.
- Projeto ESP-IDF INT8 original: mesmos tipos de arquivos.
- Parâmetros e dados de teste gerados pelo notebook, quando necessários para compilar os firmwares.
- Registro das opções efetivas de build e instruções do QEMU conforme ambiente realmente utilizado.

**Importante:** não incluir `build/`, arquivos pessoais, dados do dataset nem credenciais no repositório.

Os códigos para execução no ESP32-S3 emulado não podem ser apresentados como já publicados enquanto seus arquivos reais não estiverem presentes.

## Histórico do notebook

Esta publicação prioriza `Algoritmo(10).ipynb`, que acrescenta auditorias metodológicas à versão 9. A cópia foi mantida com os outputs originais e deve ser considerada registro da execução anterior, não uma nova execução.
