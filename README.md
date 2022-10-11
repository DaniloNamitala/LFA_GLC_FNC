### INSTRUÇÕES DE COMPILAÇÃO

Na pasta raiz do projeto, execute o seguinte comando:

``` g++ src/* -I./include -o main ```

### INSTRUÇÕES DE EXECUÇÃO

O programa deve ser chamado passando como argumentos os caminhos do arquivo de entrada
e do arquivo de saida.

Os arquivos de teste estão disponiveis dentro da pasta 'input'.

Para executar o programa com um arquivo de teste fornecido use o seguinte comando:

``` ./main input/entry1.txt output/out1.txt ```

desta forma o programa vai ler o arquivo entry1.txt na pasta input e gravar em um arquivo chamado out1.txt
na pasta output.

Para executar com o modo verbose, onde o passo a passo é mostrado no terminal adicione um argumento v ao final:

``` ./main input/entry1.txt output/out1.txt v ```
