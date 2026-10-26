# Ambiente Python utilizado nos experimentos

Este documento registra o ambiente **observado no computador do autor** ao executar o notebook `notebooks/Algoritmo.ipynb`. Foi levantado por meio de consultas ao interpretador Python, à localização dos módulos e ao gerenciador de pacotes do Ubuntu. **Não é um lockfile nem comprova, por si só, a reprodução em uma instalação limpa.**

## Sistema e bibliotecas científicas

- Sistema: **Ubuntu 22.04.5 LTS (Jammy)**.
- Interpretador do kernel: **Python 3.10.12**, `/usr/bin/python3`.
- Pacotes científicos localizados em `/usr/lib/python3/dist-packages/`, consistentes com os pacotes da distribuição.

| Biblioteca | Versão observada no Python | Pacote APT e versão observados no sistema |
| --- | --- | --- |
| Python | 3.10.12 | `python3 3.10.6-1~22.04.1` (metapacote) |
| NumPy | 1.21.5 | `python3-numpy 1:1.21.5-1ubuntu22.04.1` |
| Pandas | 1.3.5 | `python3-pandas 1.3.5+dfsg-3` |
| Scikit-learn | 0.23.2 | `python3-sklearn 0.23.2-5ubuntu6` |
| SciPy | 1.8.0 | `python3-scipy 1.8.0-1exp2ubuntu1` |
| Matplotlib | 3.5.1 | `python3-matplotlib 3.5.1-2build1` |

A versão de execução do Python (3.10.12) não precisa coincidir com a versão do metapacote `python3` no APT. As versões do pacote `python3.10-minimal` e do executável do sistema não foram inventariadas.

## Componentes de execução de notebooks

| Componente | Versão observada | Localização |
| --- | --- | --- |
| `ipykernel` | 6.29.5 | `~/.local/lib/python3.10/site-packages/` |
| `jupyter_client` | 8.6.3 | `~/.local/lib/python3.10/site-packages/` |
| `notebook` | Não encontrado neste interpretador | — |

Os componentes Jupyter acima estão no diretório de pacotes do usuário. **A localização não comprova qual instalador foi usado**. O notebook foi utilizado pelo VS Code, que pode operar com `ipykernel` sem o servidor clássico `notebook`.

## Preparar um ambiente semelhante (Ubuntu 22.04)

Para reproduzir a escolha de bibliotecas do sistema, em uma instalação Ubuntu 22.04:

```bash
sudo apt-get update
sudo apt-get install -y python3 python3-numpy python3-pandas python3-sklearn python3-scipy python3-matplotlib build-essential
```

Esse comando é uma **orientação inicial**, e não garante as versões exatas acima: repositórios, atualizações de segurança e espelhos do APT podem fornecer versões diferentes. **Não substitua silenciosamente** o ambiente histórico por pacotes recentes nem use `pip install -r requirements.txt` como se isso recriasse as versões do experimento. Para reprodução estrita, será necessário congelar um snapshot de repositórios APT ou construir/testar uma imagem de ambiente que instale precisamente as versões históricas, com o kernel Jupyter também controlado.

Abra `notebooks/Algoritmo.ipynb` no VS Code utilizando o interpretador correto. Se houver dificuldade em inicializar o kernel, configure o `ipykernel` compatível no ambiente de execução antes de prosseguir; os comandos de instalação desse componente ainda precisam ser testados em ambiente limpo.

## Conferir versões no interpretador usado pelo notebook

```python
import sys
from importlib.metadata import version
import numpy, pandas, sklearn, scipy, matplotlib

print("Python:", sys.version)
print("Executável:", sys.executable)
for nome, modulo in [
    ("NumPy", numpy), ("Pandas", pandas), ("Scikit-learn", sklearn),
    ("SciPy", scipy), ("Matplotlib", matplotlib)
]:
    print(nome, modulo.__version__, modulo.__file__)
for nome in ["ipykernel", "jupyter_client"]:
    try:
        print(nome, version(nome))
    except Exception as erro:
        print(nome, "indisponível:", erro)
```

Para conferir os pacotes APT:

```bash
dpkg-query -W -f='${binary:Package} ${Version}\n' \
  python3 python3-numpy python3-pandas python3-sklearn \
  python3-scipy python3-matplotlib
```

## Estado da reprodutibilidade

- **Registrado:** versões Python e de bibliotecas observadas no ambiente experimental original.
- **Registrado:** localização dos módulos do kernel e versões `ipykernel`/`jupyter_client`.
- **Pendente:** recriar essas dependências em um ambiente independente, executar o notebook do início ao fim e verificar resultados e arquivos gerados.
- **Pendente:** controlar dependências transitivas, sementes/estado dos geradores aleatórios e snapshots do APT, se a meta for reprodução exata em novas máquinas.

Para a origem dos dados, etapas do notebook e reprodução do firmware no QEMU, consulte `REPRODUCAO.md` e `REPRODUCAO_QEMU.md`.
