# T4, Parte III: estruturas de dados e API do grafo

## Estruturas de dados

### 1. Busca de neurônio por id (`individuo.c`)

**Problema:** `indivíduo_neurônio_com_id` e `indivíduo_sinapse_com_ids` acham um neurônio percorrendo a rede inteira. São usadas em `compatibilidade`, em `indivíduo_mistura_sinapses` e, indiretamente, na especiação (`geração_especia_indivíduo`).

**Por que é problema:** cada comparação entre duas redes custa (sinapses de uma) × (neurônios da outra), e a especiação compara cada indivíduo novo com o campeão de várias espécies. Hipótese: é um dos trechos mais pesados do treinamento.

**Alternativa:** guardar em cada indivíduo uma tabela de dispersão `id → posição` (uma ABB também serviria).

**Por que melhora:** a busca cai de O(n) para O(1) em média, e comparar duas redes passa a custar mais ou menos só o número de sinapses.

### 2. Tabela de inovações (`individuo.c`)

**Problema:** as inovações ficam num vetor global de 10000 posições, e `id_neurônio_entre` o percorre do início ao fim. Se o vetor enche, o código imprime um erro e devolve um id novo sem registrar.

**Por que é problema:** a busca fica mais lenta com o tempo, e depois do limite a mesma inovação pode receber ids diferentes, o que estraga a comparação entre redes. Não vi estourar nas execuções que fiz.

**Alternativa:** tabela de dispersão dinâmica com chave `(id_origem, id_destino)` (uma ABB com esse par como chave também serviria).

**Por que melhora:** busca em O(1) em média (ou O(log n) na ABB) e sem limite fixo.

### 3. Vetores de tamanho máximo (`especie.c`, `geracao.c`)

**Problema:** cada espécie tem um vetor de 500 indivíduos, e a geração tem vetores de 500 indivíduos e 500 espécies (`config.h`). Cada espécie reserva uns 4 KB mesmo com um único indivíduo.

**Por que é problema:** nas primeiras gerações das execuções que fiz havia cerca de 489 espécies, ou seja, uns 2 MB para quase nada, e se algum limite estourar o `assert` derruba o programa. Além disso, o sorteio `aleatório_p` favorece as primeiras posições, então o código depende de os vetores estarem ordenados, um combinado implícito entre `especie.c` e `geracao.c`.

**Alternativa:** vetor dinâmico (com `realloc`) ou lista encadeada, escondido em operações do TAD.

**Por que melhora:** a memória fica proporcional ao uso, sem limite fixo, e a garantia de ordem fica num lugar só.

### 4. Sorteio de sinapse por tentativas (`individuo.c`)

**Problema:** `mutação_divide_sinapse` e `mutação_inverte_habilitação_sinapse` escolhem uma sinapse sorteando um par de neurônios e testando se existe sinapse entre eles, repetindo até 50 vezes e desistindo depois disso. (Há um laço igual em `mutação_remove_sinapse`, mas essa função não é chamada em lugar nenhum, e o próprio código a marca como não usada.)

**Por que é problema:** o número de pares possíveis cresce com o quadrado do número de neurônios, e a rede tem poucas sinapses perto disso, então muitas tentativas tendem a falhar (não medi a proporção). Se as 50 tentativas acabam, a mutação simplesmente não acontece.

**Alternativa:** cada indivíduo guardar um vetor dinâmico com as suas sinapses e a mutação sortear direto uma posição dele. Outra saída, sem duplicar dados, é a API do grafo oferecer a aresta de posição k (ver sugestão 3 da API).

**Por que melhora:** o sorteio sempre cai numa sinapse que existe, e nenhuma mutação é perdida. O custo do vetor é manter a lista atualizada a cada sinapse inserida ou removida.

### 5. Indivíduos guardados em dois lugares (`geracao.c`, `especie.c`)

**Problema:** cada indivíduo está no vetor da geração e também no vetor da sua espécie (o comentário no começo de `geracao.c` admite isso). `geração_destrói` libera os indivíduos pelo vetor da geração, e `espécie_destrói` só libera a espécie.

**Por que é problema:** são duas listas para manter coerentes, e quem libera a memória é um combinado que o código não garante. Quem mexer em um dos lados pode causar vazamento ou liberação dupla.

**Alternativa:** guardar os indivíduos só dentro das espécies, e a geração guardar apenas as espécies. Para jogar ou achar o melhor, a geração percorre as espécies.

**Por que melhora:** cada indivíduo passa a ter um único dono, com um lugar só para inserir e um só para liberar.

### 6. Posição livre no jogo (`jogo.c`)

**Problema:** `posição_livre` usa `fila_contém`, que varre a fila dos obstáculos e a do corpo da cobra, comparando posição por posição. Ela roda 3 vezes por passo em `jogo_preenche_saídas`, mais na colisão e no sorteio de posição.

**Por que é problema:** a fila serve para guardar a ordem da cobra, mas é ruim para perguntar "tem algo aqui?". Hipótese: isso pesa no tempo, já que cada indivíduo joga 3 partidas de até 3000 passos por geração.

**Alternativa:** manter a fila e acrescentar uma matriz de ocupação da tela, atualizada quando a cobra anda.

**Por que melhora:** a pergunta passa de O(tamanho da fila) para O(1), ao custo de um pouco de memória.

## Sobre o meu grafo

Usei matriz de adjacência. Nas redes do treinamento (esparsas), percorrer os vizinhos de um nó custa O(n) mesmo com poucas sinapses, e `g_insere_nó` recopia a matriz n × n. Uma lista de adjacência seria mais barata nisso. Mantive a matriz porque as redes são pequenas e ela deixa o acesso por (origem, destino) em O(1) e a remoção de nós simples.

## Sugestões para a API do grafo

### 1. Consulta com cursor próprio

**Problema:** só existe uma consulta de arestas por vez, porque o estado dela fica dentro do grafo.

**Por que é problema:** não dá para fazer percursos aninhados, e meu `g_tem_ciclo` e `g_ordem_topológica` usam consultas por dentro, então corromperiam uma consulta em andamento no mesmo grafo.

**Alternativa:** `g_arestas_que_partem` e `g_arestas_que_chegam` devolverem um cursor (uma struct pequena) que o usuário passa para `g_próxima_aresta`.

**Por que melhora:** várias consultas podem coexistir sem se atrapalhar.

### 2. Percorrer todas as arestas

**Problema:** não há como percorrer todas as arestas do grafo. O próprio professor reclama disso num comentário em `indivíduo_mistura_sinapses`.

**Por que é problema:** `compatibilidade`, `indivíduo_grava`, `indivíduo_mistura_sinapses` e `mutação_altera_valor_sinapses` repetem um laço sobre os nós com outro sobre as sinapses de cada um.

**Alternativa:** uma operação que percorra todas as arestas (origem, destino e valor).

**Por que melhora:** um laço só, com menos código repetido.

### 3. Número de arestas e aresta de posição k

**Problema:** o grafo não informa quantas arestas tem nem dá acesso à k-ésima. Por isso o treinador sorteia pares de neurônios e testa se há sinapse entre eles (item 4 das estruturas de dados).

**Por que é problema:** o sorteio por tentativas falha com frequência e pode desistir sem fazer a mutação.

**Alternativa:** `g_num_arestas(self)` e uma operação que devolva a aresta de posição k (com origem, destino e valor).

**Por que melhora:** basta sortear um número de 0 até o total e pegar essa aresta, que sempre existe. Com a minha matriz de adjacência, achar a k-ésima aresta exige percorrer a matriz, então com a API nova seria preciso manter um contador ou um índice de arestas.

### 4. Perguntar se existe caminho antes de inserir

**Problema:** `rede_insere_sinapse` insere a aresta, roda `g_tem_ciclo` no grafo inteiro e remove a aresta se houver ciclo. Em `indivíduo_mistura_sinapses` isso acontece para cada sinapse do filhote, mas ali ciclo é impossível, porque o filhote é um subgrafo do dominante, que já era acíclico.

**Por que é problema:** é trabalho refeito à toa, já que só a aresta nova pode criar um ciclo, e quando há ciclo a aresta é inserida só para ser apagada. Hipótese: pesa no treinamento (as redes são pequenas, então talvez pese pouco).

**Alternativa:** `g_existe_caminho(self, origem, destino)`. Uma aresta de A para B só forma ciclo se já existir caminho de B para A, então a rede testa antes de inserir. Uma operação `g_insere_aresta_sem_ciclo`, que usa essa busca por dentro, também serviria.

**Por que melhora:** a verificação acontece antes da inserção e visita só a parte do grafo alcançável a partir do destino, sem inserir e desfazer.