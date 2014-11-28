
Proposta para modificação da TRIOS (v2)
=======================================

Estão detalhadas neste documentos as modificações que gostaria de fazer para tornar a TRIOS
mais flexível e mais aberta à novas técnicas. 


Código atual  (API antiga)
------------

A API antiga é usa as funções

	*lcollec, ldecision e lisi/ltrainGG, etc *.

Cada uma das funções implementa uma parte do processo de treinamento e faz entrada e saída separadamente.

Para a aplicação são usadas as funções lpapplic`*`. Cada uma delas implementa a aplicação de um tipo específico
de operador e o usuário escolhe a função "certa".

Para criar um novo tipo de operador é necessário escrever as funções de treinamento e aplicação do zero e, possivelmente,
realizar uma nova coleta. Os parametros de cada funçao sao completamente diferentes.

Operadores dois niveis nao estao implementados.

Codigo atual (API nova)
---------------

A API nova define o tipo image_operator_t. Ela é uma interface de alto nível para a API antiga e
concentra as chamadas das tres funçoes em apenas uma (*image_operator_build*).

	typedef struct {
		int type; 
		window_t *win;
		xpl_t *collec; 
		mtm_t *decision;
	
		itv_t *bb; 
		dTree *gg; 
		apert_t *apt; 
	} image_operator_t;

Estão implementadas operações de entrada e saída, treinamento e aplicação utilizando esta estrutura. Para cada tipo
novo de operador é necessário adicionar um *case* novo em cada uma das funções. Por outro lado, a interface padrão
facilita muito a utilização dos operadores treinados. O código fica mais limpo e fácil de entender.

A implementação de operadores multi-níveis obedece a uma API *parecida* com a do image_operator_t.

Problemas no código atual
-------------------------

Existem dois grandes problemas no código atual:

1. quantidade significativa de código legado/"de testes" e que não é usado.
2. **lógica espalhada por todo o código**: a implementação de um novo tipo de operador requer a recodificação de algumas tarefas "básicas" e afeta muitos locais diferentes da biblioteca.


Proposta
--------

A minha proposta se basea em "deletar tudo" e ir adicionando somente o essencial. Na minha opinião é mais fácil pôr do que tirar. Eu também gostaria
de aproveitar e sugerir algumas mudanças estruturais importantes.

### Substituição de código específico para binário por código genérico

Eu gostaria de "jogar" fora a árvore BB. Ter duas árvores só aumenta a quantidade de código existente e dificulta a manutenção do código. Outro
fator importante é que funções codificadas para dados binários **precisam ser reescritas** para funcionarem em níveis de cinza. Isto acontece não
somente com a árvore, mas com vários outras funções (aplicação, seleção de janelas, etc).

Precisaríamos também padronizar o formato do padrão usado (int, unsigned int, float ?).

### Definição de uma estrutura que "descreve" um tipo de operador

Esta mudança reorganizaria boa parte do código da TRIOS. A ideia seria ter uma estrutura que contém ponteiros para todas as funções que podem ser
usadas por um operador (treinar, aplicar, escrever e ler do disco, etc).	

	struct operator_type {
		int type; /* para poder diferenciar os varios tipos de operadores. */
		
		void (*train)(xpl_t *data, window_t *win, void *operator_params);
		int (*apply)(int *); /* aplica para um ponto */
		
		int (*write)(char *filename);
		image_operator_t *(*read)(char *filename);
		
	};

Um tipo de operador poderia ser registrado com uma função, por exemplo *trios_register(int, operator_type *);*,
e uma função *trios_register_default();* poderia existir para incluir os operadores "padrão". Todo o código
de um operador poderia ficar mais concentrado em um só lugar e possivelmente seria mais flexível.

Com estas funções também poderia ser possível construir um operador de dois níveis (mas não mais que dois níveis).
O código multi-nível está muito complexo e poder transformá-lo em uma versão mais simples e flexível seria muito
bom também.

A sequência de chamadas seria *collec -> build -> train*. O collec fica fora do operador e sempre faz a coleta em
níveis de cinza. Tanto a binarização quanto a decisão ficam por conta de função *train* de cada operador. 