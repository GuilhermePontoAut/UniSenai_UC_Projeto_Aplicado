
# Redes neurais compactas para diagnóstico de degradação hidráulica em Edge AI

Repositório do projeto aplicado UniSENAI sobre a viabilidade de redes neurais compactas para diagnóstico de condições de sistemas hidráulicos em **Edge AI**.

> **Pesquisa em desenvolvimento.** Esta disponibilização inclui o notebook experimental. Os projetos originais **ESP-IDF FP32 e INT8 para ESP32-S3/QEMU** estão disponíveis em `firmware/`. Consulte [Reprodução no QEMU](docs/REPRODUCAO_QEMU.md) para os passos e limitações.

## Objetivo e dados

O estudo investiga qualidade preditiva, redução de características, efeitos da quantização e requisitos computacionais de duas tarefas de classificação multiclasse:

- **Bomba hidráulica:** três níveis de vazamento interno (classes 0, 1 e 2).
- **Acumulador hidráulico:** quatro níveis de condição (classes 90, 100, 115 e 130).

Dataset público: [Condition Monitoring of Hydraulic Systems — UCI](https://archive.ics.uci.edu/dataset/447/condition+monitoring+of+hydraulic+systems). São **2.205 ciclos de 60 s**, com 17 sinais e quatro estatísticas por sinal (média, desvio-padrão amostral, mínimo e máximo), totalizando **68 características**.

## Organização

- `notebooks/Algoritmo.ipynb` — versão mais recente disponibilizada pelo pesquisador como `Algoritmo(10).ipynb`, com experimentos e auditorias.
- `docs/REPRODUCAO.md` — dados, ambiente e orientações de execução.
- `docs/ESTRUTURA_E_STATUS.md` — código publicado e materiais ainda pendentes.
- `requirements.txt` — bibliotecas Python envolvidas nos experimentos.

Os gráficos e as visualizações dos experimentos estão incorporados ao notebook `notebooks/Algoritmo.ipynb`. Não é necessária uma pasta separada de imagens.

## Metodologia

1. Caracterização do conjunto de dados e extração das 68 estatísticas por ciclo.
2. Separação de desenvolvimento e teste reservado, com grupos de condições experimentais.
3. Comparação de regressão logística, árvore de decisão, Random Forest e MLP.
4. Estudo de arquiteturas MLP e redução de características.
5. Treinamento final e exportação para C++, com versões FP32 e quantização INT8.
6. Conferência das previsões C++/Python e execução do núcleo de inferência no ESP32-S3 emulado por QEMU.
7. Avaliação nos dados reservados e auditoria das estratégias de divisão.

As redes finais têm uma camada oculta de 16 neurônios: **16 entradas para bomba e 65 para acumulador**.

## Resultados de teste no computador

| Componente | Amostras de teste | F1 macro FP32 | F1 macro INT8 |
|---|---:|---:|---:|
| Bomba | 508 | 100,00% | 100,00% |
| Acumulador | 439 | 90,12% | 90,98% |

As métricas representam conjuntos reservados específicos. A validação GroupKFold do acumulador apresentou F1 macro médio de **68,82%**, evidenciando variação entre divisões e condições experimentais. Não é correto extrapolar os resultados como desempenho universal.

A quantização reduziu o armazenamento dos arranjos de parâmetros, mas não o tamanho do firmware completo nem a latência na avaliação exploratória. A versão INT8 utiliza **precisão mista**: pesos/ativações inteiros, com partes do cálculo em FP32.

## ESP32-S3 / QEMU — limitações

Os projetos C++ foram compilados com ESP-IDF v6.1 e executados em **ESP32-S3 emulado** no QEMU. Foram verificadas previsões de **6 amostras da bomba e 8 do acumulador por representação**, selecionadas do desenvolvimento. A avaliação completa de F1 ocorreu **no computador**, não no emulador.

**Não foram realizados testes em hardware físico**, nem medições de energia ou implementação da aquisição contínua dos sinais e extração de características no ESP32-S3. As latências no QEMU não representam diretamente desempenho temporal real.

### Trabalhos futuros

Como continuidade, propõe-se integrar a aquisição e o pré-processamento dos sinais no ESP32-S3 e avaliar memória, latência, consumo de energia e estabilidade em placa física. Também é importante investigar a generalização em outras condições operacionais.

## Como reproduzir

Consulte [docs/REPRODUCAO.md](docs/REPRODUCAO.md). O dataset deve ser obtido do repositório oficial da UCI.

**Referência:** Helwig, N.; Pignanelli, E.; Schütze, A. *Condition Monitoring of Hydraulic Systems*. UCI Machine Learning Repository. DOI: [10.24432/C5CW21](https://doi.org/10.24432/C5CW21).

## Uso de IA generativa

Ferramentas de IA generativa foram utilizadas como apoio à organização da documentação, à revisão textual e ao esclarecimento de dúvidas técnicas. As decisões metodológicas, a execução dos experimentos e a interpretação dos resultados permaneceram sob responsabilidade dos autores.

## Licença e citação

O artigo ainda está em preparação. Não há licença de reutilização definida nesta etapa; uma versão científica estável poderá ser publicada posteriormente com DOI.
