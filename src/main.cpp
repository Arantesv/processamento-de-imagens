#include <SDL3/SDL_main.h>
#include "app.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Uso: " << argv[0] << " caminho_da_imagem.ext\n";
        return 1;
    }

    proj1::Application app(argv[1]);
    return app.run();
}
