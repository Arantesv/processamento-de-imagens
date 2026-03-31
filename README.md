# Processamento de imagens com SDL3

## Visao geral

Este projeto implementa, em **C++11**, um software de processamento de imagen.

O programa:

- carrega uma imagem por linha de comando;
- verifica se ela ja esta em escala de cinza;
- converte imagens coloridas para escala de cinza usando a formula especificada no enunciado;
- exibe a imagem em uma janela principal;
- exibe, em uma segunda janela, o histograma da imagem atual;
- calcula media de intensidade e desvio padrao para classificar brilho e contraste;
- permite **equalizar** o histograma por meio de um botao desenhado com primitivas da SDL;
- permite **voltar para a imagem original em escala de cinza sem recarregar o arquivo**;
- salva a imagem atual como `output_image.png` ao pressionar a tecla **S**.

---

## Funcionalidades implementadas

### 1. Carregamento de imagem

- Entrada por linha de comando no formato:

```bash
./proj1 caminho_da_imagem.ext
```

- Suporte a formatos comuns por meio da `SDL_image`, como PNG, JPG e BMP.
- Tratamento de erro para:
  - arquivo inexistente;
  - caminho invalido;
  - arquivo que nao representa uma imagem valida.

### 2. Deteccao e conversao para escala de cinza

A imagem carregada e convertida internamente para `RGBA32` e, em seguida, o programa verifica se todos os pixels satisfazem:

```text
R = G = B
```

Se a imagem for colorida, ela e convertida para escala de cinza usando exatamente a formula exigida pelo enunciado:

```text
Y = 0.2125 * R + 0.7154 * G + 0.0721 * B
```

A versao em escala de cinza passa a ser a base para todas as operacoes seguintes.

### 3. GUI com duas janelas

- **Janela principal**
  - exibe a imagem atualmente em processamento;
  - seu tamanho e igual ao tamanho da imagem;
  - e centralizada ao iniciar.

- **Janela secundaria**
  - possui tamanho fixo;
  - exibe o histograma;
  - exibe as informacoes analiticas da imagem;
  - exibe o botao de operacao.


### 4. Histograma e analise

A janela secundaria mostra:

- histograma da imagem atual (original em cinza ou equalizada);
- media de intensidade;
- classificacao de brilho: `escura`, `media` ou `clara`;
- desvio padrao das intensidades;
- classificacao de contraste: `baixo`, `medio` ou `alto`.

#### Criterio de classificacao adotado

Como o PDF-base explica o conceito de imagens escuras/claras e de baixo/alto contraste, mas nao fixa limiares numericos obrigatorios, foi adotado um criterio objetivo e simples sobre a faixa de 8 bits (`0..255`):

- **Brilho**
  - `escura`: media < 85
  - `media`: 85 <= media <= 170
  - `clara`: media > 170

- **Contraste**
  - `baixo`: desvio padrao < 42.5
  - `medio`: 42.5 <= desvio padrao <= 85
  - `alto`: desvio padrao > 85

Esse critério divide aproximadamente a faixa util em tres regioes para facilitar avaliacao visual e reproducibilidade.

### 5. Equalizacao do histograma

O botao da janela secundaria alterna entre dois estados:

- `Equalizar`
- `Ver original`

Ao clicar:

1. o programa equaliza o histograma da imagem em escala de cinza;
2. atualiza a imagem mostrada na janela principal;
3. recalcula e redesenha o histograma na janela secundaria.

Ao clicar novamente:

1. a exibição volta para a imagem original em escala de cinza;
2. o histograma e as informacoes sao restaurados;
3. nenhum recarregamento do arquivo original é feito.

### 6. Salvamento

Ao pressionar a tecla **S**, o programa salva a imagem atualmente exibida em:

```text
output_image.png
```

Se o arquivo ja existir, ele é sobrescrito.

---

## Estrutura do projeto

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

## Organizacao do codigo

### `src/main.cpp`

- valida argumentos da linha de comando;
- instancia a aplicacao;
- inicia o loop principal.

### `src/app.cpp`

Responsavel por:

- inicializar SDL e SDL_ttf;
- carregar imagem;
- criar janelas e renderizadores;
- gerenciar eventos de teclado e mouse;
- alternar entre imagem original em cinza e imagem equalizada;
- atualizar textura e painel de informacoes;
- salvar a imagem;
- encerrar recursos corretamente.

### `src/image_processing.cpp`

Responsavel por:

- carregar imagem com `SDL_image`;
- converter para `RGBA32`;
- detectar se a imagem ja e em escala de cinza;
- gerar imagem em escala de cinza pela formula do projeto;
- calcular histograma;
- calcular media e desvio padrao;
- classificar brilho e contraste;
- gerar imagem equalizada.

### `src/ui.cpp`

Responsavel por:

- renderizar textos com `SDL_ttf`;
- desenhar o botao;
- desenhar o histograma com escala proporcional.

---

## Dependencias

### Windows (MSYS2 UCRT64)

1. Instale o MSYS2:
https://www.msys2.org/

2. Abra o terminal:
MSYS2 UCRT64

3. Atualize o sistema:
pacman -Syu

(Se pedir, feche e abra o terminal novamente e rode o comando outra vez)

4. Instale as dependencias:
pacman -S --needed \
mingw-w64-ucrt-x86_64-toolchain \
mingw-w64-ucrt-x86_64-cmake \
mingw-w64-ucrt-x86_64-ninja \
mingw-w64-ucrt-x86_64-sdl3 \
mingw-w64-ucrt-x86_64-sdl3-image \
mingw-w64-ucrt-x86_64-sdl3-ttf

---

### Linux (Ubuntu / WSL)

Instale as dependencias:

sudo apt update
sudo apt install build-essential cmake ninja-build pkg-config \
libsdl3-dev libsdl3-image-dev libsdl3-ttf-dev

---

## Compilacao

### Metodo 1 — CMake (recomendado)

cd /c/caminho/para/processamento-de-imagens
mkdir build
cd build
cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

Executavel gerado:
build/proj1.exe

### Metodo 2 — Makefile (g++ direto)

Este metodo utiliza diretamente o compilador g++.

make

Executavel gerado:
./proj1

---

## Execucao

./proj1.exe "/c/caminho/para/imagem.png"

Exemplo:
./proj1.exe "/c/Users/Usuario/imagem.png"

---

## Como o histograma e calculado

Para uma imagem em tons de cinza de 8 bits, cada pixel possui intensidade entre `0` e `255`.

O programa monta um vetor com 256 posicoes, no qual:

- a posicao `i` armazena quantos pixels possuem intensidade `i`.

Depois disso:

- a **media** e calculada com base nas frequencias;
- o **desvio padrao** mede a dispersao das intensidades;
- a **equalizacao** e feita pela CDF (funcao de distribuicao acumulada), convertendo cada intensidade original em uma nova intensidade redistribuida.

---

## Tratamento de erros

O programa emite mensagens claras no terminal para casos como:

- uso incorreto da linha de comando;
- falha ao abrir imagem;
- falha ao criar janelas;
- falha ao criar renderers;
- falha ao carregar fonte;
- falha ao salvar imagem.

---

## Contribuicao de cada integrante

- **Vitor Arantes Vendramini / 10417759**
  - implementacao da estrutura inicial do projeto;
  - configuracao do build com CMake e Makefile;
  - desenvolvimento da classe principal da aplicacao (Application);
  - criacao das janelas, renderers e exibicao da imagem;
  - integracao final do sistema;
  - implementacao da equalizacao do histograma;
  - implementacao da reversao para imagem original em escala de cinza;
  - implementacao do salvamento da imagem com tecla S.

---

- **Matheus Queiroz Gregorin / 10418143**
  - implementacao do carregamento de imagem com SDL_image;
  - conversao da imagem para o formato RGBA32;
  - deteccao de imagens em escala de cinza;
  - implementacao da conversao para escala de cinza utilizando a formula do projeto;
  - implementacao do calculo do histograma;
  - implementacao do calculo da media de intensidade;
  - implementacao do desvio padrao;
  - classificacao de brilho (escura, media, clara);
  - classificacao de contraste (baixo, medio, alto).

---

- **Pedro Henrique C. Guimarães / 10417477**
  - implementacao da interface grafica da janela secundaria;
  - desenho do histograma;
  - renderizacao de textos com SDL_ttf;
  - implementacao do botao interativo (desenho e estados visuais);
  - tratamento de eventos de mouse (hover, clique e release);
  - atualizacao dinamica do painel de informacoes;
  - sincronizacao entre janela principal e secundaria.

---