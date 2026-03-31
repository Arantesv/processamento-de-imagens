# Processamento de Imagens com SDL3

## Visão Geral

Este projeto implementa, em **C++11**, um software de processamento de imagem.

O programa:

- carrega uma imagem por linha de comando;
- verifica se ela já está em escala de cinza;
- converte imagens coloridas para escala de cinza usando a fórmula especificada no enunciado;
- exibe a imagem em uma janela principal;
- calcula a média de intensidade e o desvio padrão para classificar brilho e contraste;
- permite **equalizar** o histograma por meio de um botão desenhado com primitivas da SDL;
- permite **voltar para a imagem original em escala de cinza sem recarregar o arquivo**;
- salva a imagem atual como `output_image.png` ao pressionar a tecla **S**;
- exibe, em uma segunda janela, o histograma da imagem atual.

---

## Funcionalidades Implementadas

### 1. Carregamento de Imagem

- Entrada por linha de comando no formato:

```bash
./proj1 caminho_da_imagem.ext
```

- Suporte a formatos comuns por meio da biblioteca `SDL_image`, como PNG, JPG e BMP.
- Tratamento de erro para:
  - arquivo inexistente;
  - caminho inválido;
  - arquivo que não representa uma imagem válida.

### 2. Detecção e Conversão para Escala de Cinza

A imagem carregada é convertida internamente para `RGBA32` e, em seguida, o programa verifica se todos os pixels satisfazem a condição:

```text
R = G = B
```

Se a imagem for colorida, ela é convertida para escala de cinza usando exatamente a fórmula exigida pelo enunciado:

```text
Y = 0.2125 * R + 0.7154 * G + 0.0721 * B
```

A versão em escala de cinza passa a ser a base para todas as operações seguintes.

### 3. Interface Gráfica (GUI) com Duas Janelas

- **Janela Principal**
  - exibe a imagem atualmente em processamento;
  - possui tamanho igual ao da imagem;
  - é centralizada ao iniciar.

- **Janela Secundária**
  - possui tamanho fixo;
  - exibe:
    - o histograma;
    - as informações analíticas da imagem;
    - o botão de operação.

### 4. Histograma e Análise

A janela secundária mostra:

- histograma da imagem atual (original em cinza ou equalizada);
- média de intensidade;
- classificação de brilho: `escura`, `média` ou `clara`;
- desvio padrão das intensidades;
- classificação de contraste: `baixo`, `médio` ou `alto`.

#### Critério de Classificação Adotado

Como o PDF-base explica o conceito de imagens escuras/claras e de baixo/alto contraste, mas não fixa limiares numéricos obrigatórios, foi adotado um critério objetivo e simples sobre a faixa de 8 bits (`0..255`):

- **Brilho**
  - `escura`: média < 85
  - `média`: 85 <= média <= 170
  - `clara`: média > 170

- **Contraste**
  - `baixo`: desvio padrão < 42.5
  - `médio`: 42.5 <= desvio padrão <= 85
  - `alto`: desvio padrão > 85

Esse critério divide aproximadamente a faixa útil em três regiões para facilitar a avaliação visual e a reprodutibilidade.

### 5. Equalização do Histograma

O botão da janela secundária alterna entre dois estados:

- `Equalizar`
- `Ver original`

Ao clicar:

1. o programa equaliza o histograma da imagem em escala de cinza;
2. atualiza a imagem mostrada na janela principal;
3. recalcula e redesenha o histograma na janela secundária.

Ao clicar novamente:

1. a exibição volta para a imagem original em escala de cinza;
2. o histograma e as informações são restaurados;
3. nenhum recarregamento do arquivo original é feito.

### 6. Salvamento

Ao pressionar a tecla **S**, o programa salva a imagem atualmente exibida em:

```text
output_image.png
```

Se o arquivo já existir, ele será sobrescrito.

---

## Estrutura do Projeto

```text
processamento-de-imagens/
├── CMakeLists.txt
├── Makefile
├── README.md
├── include/
│   ├── app.hpp
│   ├── image_processing.hpp
│   └── ui.hpp
└── src/
    ├── app.cpp
    ├── image_processing.cpp
    ├── main.cpp
    └── ui.cpp
```

---

## Organização do Código

### `src/main.cpp`

- valida os argumentos da linha de comando;
- instancia a aplicação;
- inicia o loop principal.

### `src/app.cpp`

Responsável por:

- inicializar SDL e SDL_ttf;
- carregar a imagem;
- criar janelas e renderizadores;
- gerenciar eventos de teclado e mouse;
- alternar entre imagem original em cinza e imagem equalizada;
- atualizar textura e painel de informações;
- salvar a imagem;
- encerrar recursos corretamente.

### `src/image_processing.cpp`

Responsável por:

- carregar a imagem com `SDL_image`;
- converter para `RGBA32`;
- detectar se a imagem já é em escala de cinza;
- gerar imagem em escala de cinza pela fórmula do projeto;
- calcular o histograma, a média e o desvio padrão;
- classificar o brilho e o contraste;
- gerar a imagem equalizada.

### `src/ui.cpp`

Responsável por:

- renderizar os textos com `SDL_ttf`;
- desenhar o botão;
- desenhar o histograma com escala proporcional.

---

## Dependências

### Windows (MSYS2 UCRT64)

1. Instale o [MSYS2](https://www.msys2.org/).
2. Abra o terminal **MSYS2 UCRT64**.
3. Atualize o sistema executando o comando abaixo. *(Se solicitado, feche o terminal, abra-o novamente e rode o comando mais uma vez).*

```bash
pacman -Syu
```

4. Instale as dependências necessárias:

```bash
pacman -S --needed \
mingw-w64-ucrt-x86_64-toolchain \
mingw-w64-ucrt-x86_64-cmake \
mingw-w64-ucrt-x86_64-ninja \
mingw-w64-ucrt-x86_64-sdl3 \
mingw-w64-ucrt-x86_64-sdl3-image \
mingw-w64-ucrt-x86_64-sdl3-ttf
```

---

### Linux (Ubuntu / WSL)

Instale as dependências executando os comandos a seguir no terminal:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build pkg-config \
libsdl3-dev libsdl3-image-dev libsdl3-ttf-dev
```

---

## Compilação

### Método 1 — CMake (Recomendado)

```bash
cd /c/caminho/para/processamento-de-imagens
mkdir build
cd build
cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Executável gerado: `build/proj1.exe`

### Método 2 — Makefile (Compilação Direta)

Este método utiliza diretamente o compilador `g++`.

```bash
make
```

Executável gerado: `./proj1`

---

## Execução

```bash
./proj1.exe "/c/caminho/para/imagem.png"
```

**Exemplo:**
```bash
./proj1.exe "/c/Users/Usuario/imagem.png"
```

---

## Como o Histograma é Calculado

Para uma imagem em tons de cinza de 8 bits, cada pixel possui uma intensidade entre `0` e `255`.

O programa monta um vetor com 256 posições, no qual a posição `i` armazena quantos pixels possuem a intensidade `i`.

Depois disso:

- a **média** é calculada com base nas frequências;
- o **desvio padrão** mede a dispersão das intensidades;
- a **equalização** é feita pela CDF (Função de Distribuição Acumulada), convertendo cada intensidade original em uma nova intensidade redistribuída.

---

## Tratamento de Erros

O programa emite mensagens claras no terminal para casos como uso incorreto da linha de comando ou falhas ao:

- abrir a imagem;
- criar janelas ou *renderers*;
- carregar a fonte;
- salvar a imagem.

---

## Contribuição de Cada Integrante

- **Vitor Arantes Vendramini / 10417759**
  - estrutura inicial do projeto;
  - configuração do *build* com CMake e Makefile;
  - desenvolvimento da classe principal da aplicação (`Application`);
  - criação das janelas, *renderers* e exibição da imagem;
  - integração final do sistema;
  - equalização do histograma;
  - reversão para a imagem original em escala de cinza;
  - salvamento da imagem com a tecla **S**.

---

- **Matheus Queiroz Gregorin / 10418143**
  - carregamento de imagem com `SDL_image`;
  - conversão da imagem para o formato `RGBA32`;
  - detecção de imagens em escala de cinza;
  - conversão para escala de cinza utilizando a fórmula do projeto;
  - cálculos do histograma, média de intensidade e desvio padrão;
  - classificação de brilho (escura, média, clara) e de contraste (baixo, médio, alto).

---

- **Pedro Henrique C. Guimarães / 10417477**
  - interface gráfica da janela secundária;
  - desenho do histograma;
  - renderização de textos com `SDL_ttf`;
  - botão interativo (desenho e estados visuais);
  - tratamento de eventos de mouse (*hover*, clique e *release*);
  - atualização dinâmica do painel de informações;
  - sincronização entre a janela principal e a secundária.