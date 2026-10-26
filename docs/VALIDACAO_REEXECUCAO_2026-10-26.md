# Verificação de reexecução do notebook — 2026-10-26

Esta verificação foi feita sobre o arquivo `Algoritmo_reproducao.ipynb`, cópia local do notebook publicado, executada pelo pesquisador no ambiente original Ubuntu. Não equivale a uma reprodução independente em uma máquina limpa.

## Inspeção do arquivo reexecutado

- 188 células no arquivo: 102 células Markdown e 86 células de código.
- 85 células de código com instruções executadas em sequência (contadores 1 a 85).
- A última célula de código está vazia e, por isso, sem contador de execução.
- **Zero saídas do tipo `error`**.
- O conteúdo-fonte das 86 células de código é **idêntico** ao de `notebooks/Algoritmo.ipynb` na branch `main` no momento da comparação (comparação integral do texto-fonte; não apenas resultados).
- Na etapa de treinamento/validação correspondente à célula de índice 132, há **dois avisos `ConvergenceWarning`** do Scikit-learn, informando que o treinamento chegou a `max_iter=1200` sem satisfazer o critério de convergência. São avisos, não exceções; não devem ser apagados do registro.

## Resultados reproduzidos

| Medida | FP32 | INT8 |
|---|---:|---:|
| Bomba, teste reservado (508 ciclos), F1 macro | 100,00% | 100,00% |
| Acumulador, teste reservado (439 ciclos), F1 macro | 90,12% | 90,98% |
| Acumulador, teste reservado, acurácia | 91,57% | 92,26% |
| Acumulador, divergências FP32 × INT8 (439 ciclos) | \- | 13 |

Outras verificações:

- Validação GroupKFold do acumulador, cinco dobras: F1 macro médio **68,82%**.
- Inferência FP32 em C++ comparada ao modelo de referência: **0 divergências de classificação** no teste reservado.
- Comparação de C++ FP32 com o modelo de referência no desenvolvimento: **100% de coincidência** nas duas tarefas.
- Comparação de C++ INT8 com a referência NumPy INT8 no desenvolvimento: **100% de coincidência** nas duas tarefas.
- Calibração INT8: bomba P100; acumulador P99,9. Na etapa de calibração do acumulador, **5 divergências em 1.766 amostras** de desenvolvimento após P99,9, contra 139 em P100.

### Latências C++ nativas reproduzidas no PC

Medianas do arquivo reexecutado, em µs/inferência:

| Tarefa | FP32 | INT8 |
|---|---:|---:|
| Bomba | 0,1035 | 0,1824 |
| Acumulador | 0,3458 | 0,6600 |

As latências históricas registradas no notebook anterior eram, respectivamente, 0,1134; 0,1871; 0,3661 e 0,6473 µs. **Medidas temporais variam entre execuções**; não são um critério de igualdade exata. O desempenho de classificação e os indicadores científicos principais foram reproduzidos.

## Estado da reprodutibilidade

1. **Notebook Python:** reexecutado no ambiente original, sem saídas de erro; resultados principais confirmados. O arquivo original publicado não foi alterado.
2. **Firmwares ESP-IDF FP32 e INT8:** os dois foram compilados/executados pelo pesquisador no QEMU, com 6/6 classificações da bomba e 8/8 do acumulador em cada firmware, verificados por logs locais `artefatos_reproducao/qemu/`. Esses logs locais não estão publicados.
3. **Reprodução independente:** ainda não demonstrada em uma máquina/ambiente limpo, seguindo apenas as instruções públicas.
4. **Congelamento de ambiente:** o inventário do Ubuntu e dos pacotes utilizados está em `AMBIENTE_PYTHON_ORIGINAL.md`; ainda não existe lockfile/snapshot completo que garanta restauração exata da instalação histórica.

**Limites:** o conjunto de teste completo foi avaliado no PC; QEMU verificou um subconjunto de desenvolvimento. Não foram realizados testes de inferência em ESP32-S3 físico.
