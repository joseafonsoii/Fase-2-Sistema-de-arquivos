
  O c�digo disponibilizado funciona por omiss�o com a implementa��o das tarefas nativas - pthreads.

  Para usar as tarefas em modo utilizador devem "descomentar" a linha referente ao uso das pthreads 
no ficheiro include/config.h (no fim do ficheiro).

  Para que a 2� parte do trabalho funcione com as tarefas em modo utilizador (sthreads) com o escalonamento 
pedido na 1� parte do projecto, ser� necess�rio o seguinte:

- Descompactar o pacote da biblioteca snfs dado por n�s;

- Copiar todos os ficheiros *.c e *.h por voc�s alterados para as directorias respectivas tendo em aten��o o seguinte:
  - Foram feitas altera��es nos ficheiros sthread_pthreads.c, sthread_pthreads.h e sthread.c. 
    Caso tenham alterado algum destes ficheiros t�m que ter cuidado ao fazer o merge. Basicamente o que estes ficheiros 
    t�m a mais em rela��o ao c�digo disponibilizado para a 1� parte � a implementa��o dos monitores e do sleep nas pthreads.
  - o ficheiro include/config.h foi tamb�m alterado para permitir o undefine da vari�vel USE_PTHREADS.

- A Makefiles foram tamb�m alteradas em rela��o ao que foi dado para a 1� parte do projecto. Ter em aten��o estas 
  altera��es ao fazer o merge das Makefiles:
  - Na Makefile que est� na raiz da biblioteca foram acrescentadas as directorias referentes ao snfs
  - Na Makefile do sthread_lib foi acrescentado o ficheiro sthread_pthread e acrescentado tamb�m a defini��o do USE_PTHREADS.

======================================================================================================
 
 Para o lan�amento da aplica��o servidor:
   1) Executa do comando make na directoria snfs+sthreads
   2) muda para a directoria snfs_server
   3) lan�ar na linha comandos ./server  (pode ser tamb�m ./server <io_delay> , io_delay � um inteiro positivo)


  Os testes devem ser descompactados na directoria snfs+sthreads e uma vez compilados (comando make) 
devem  ser executados num terminal depois de que o servidor j� estiver a executar noutro terminal.
 Para cada teste � preciso executar de novo o servidor no respectivo terminal.


   