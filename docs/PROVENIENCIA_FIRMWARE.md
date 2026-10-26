# Proveniência dos projetos ESP-IDF

Foram recebidos dois diretórios de fonte compilável em `firmwares_esp32s3_qemu.tar.gz`: `mlp_esp32s3` (FP32) e `mlp_esp32s3_int8` (INT8). O manifesto `firmware/SHA256_ORIGINAIS.json` guarda o hash SHA-256 do arquivo compactado e o de cada arquivo do projeto incluído.

Os arquivos essenciais de cada firmware foram copiados **sem modificações**. A distribuição omitiu apenas arquivos não utilizados na compilação ou duplicados: exemplo `hello_world_main.c`, exemplo de testes `pytest_hello_world.py`, README genérico `Hello World`, `sdkconfig.ci` vazio e cópia `sdkconfig.backup` idêntica ao `sdkconfig`. Para consultar esses arquivos históricos, recorra ao arquivo compactado originalmente enviado.

Os códigos contêm amostras de teste funcional derivadas do conjunto público UCI e constantes de modelos geradas no contexto deste estudo. Não são parâmetros provenientes de um firmware proprietário.

O objetivo deste pacote é permitir que terceiros recompilem **os mesmos fontes**, testem os 14 exemplos no QEMU e analisem o comportamento em suas próprias máquinas, sem confundir reprodução de inferência emulada com generalização para hardware real.

Para os detalhes das diferenças de build e mensagens desatualizadas, consulte `docs/REPRODUCAO_QEMU.md`.
