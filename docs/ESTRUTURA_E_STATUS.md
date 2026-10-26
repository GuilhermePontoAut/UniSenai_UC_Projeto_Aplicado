# Estado da publicação

## Incluído agora

- `notebooks/Algoritmo.ipynb`: cópia integral, sem alterar células nem resultados salvos, da última versão fornecida (`Algoritmo(10).ipynb`).
- `README.md`, `requirements.txt` e `.gitignore`.
- Documentação do dataset e do processo experimental.
- `docs/AMBIENTE_PYTHON_ORIGINAL.md`: inventário do Ubuntu 22.04.5, Python 3.10.12, pacotes científicos do APT e componentes Jupyter observados no ambiente do pesquisador. A reconstrução desse ambiente em uma instalação limpa ainda não foi validada.
- `docs/VALIDACAO_REEXECUCAO_2026-10-26.md`: auditoria do arquivo `Algoritmo_reproducao.ipynb` executado no Ubuntu do pesquisador (85 células com código executadas, nenhum erro registrado, resultados principais reproduzidos).

## Projetos embarcados publicados e validação pendente

- Projeto ESP-IDF FP32 original incluído em `firmware/mlp_esp32s3/`.
- Projeto ESP-IDF INT8 original incluído em `firmware/mlp_esp32s3_int8/`.
- Parâmetros e amostras de verificação necessários já estão nos cabeçalhos C++ versionados.
- `docs/REPRODUCAO_QEMU.md` documenta opções, comandos, diferenças de configuração e limites. Falta validar o processo inteiro em ambiente limpo.

**Importante:** não incluir `build/`, arquivos pessoais, dados do dataset nem credenciais no repositório.

Os códigos originais já estão publicados. O pesquisador executou os scripts no Ubuntu e confirmou 6/6 previsões da bomba e 8/8 do acumulador em ambos os firmwares no QEMU. A reprodução independente em instalação limpa continua pendente; os logs locais não foram adicionados ao repositório.

## Histórico do notebook

Esta publicação prioriza `Algoritmo(10).ipynb`, que acrescenta auditorias metodológicas à versão 9. A cópia foi mantida com os outputs originais e deve ser considerada registro da execução anterior, não uma nova execução.
