# Turbo Dash - PSP Edition

Jogo de plataforma rápido, **100% original** (personagem, nome e conteúdo
próprios) — apenas inspirado no *gênero* de plataforma clássica, misturando
duas ideias populares:

- **Estilo "pisar em inimigo":** encoste em cima de um inimigo (`e`) pra
  derrotá-lo; encoste de lado e você perde uma vida.
- **Estilo "pads de velocidade / molas":** pise nos trechos marcados com
  `>` pra ganhar um boost de velocidade, e no `W` pra ser lançado bem mais
  alto por uma mola.

Nenhum personagem, nome, sprite, música ou texto de terceiros foi usado —
é um jogo novo, só puxando a receita do gênero.

## Estrutura do projeto

```
turbo-dash-psp/
├── main.c                       # código do jogo
├── Makefile                     # receita de compilação (PSPSDK)
├── README.md                    # este arquivo
└── .github/workflows/build.yml  # compila sozinho no GitHub
```

## Como gerar o EBOOT.PBP (sem PC)

1. Crie um repositório novo no GitHub.
2. Suba estes 4 arquivos/pastas mantendo a mesma estrutura acima.
3. Vá na aba **Actions** do repositório e espere o ✅ verde.
4. Baixe o artefato **TurboDash-EBOOT** (contém o `EBOOT.PBP`).

## Como jogar

- **PSP real com CFW:** coloque a pasta com `EBOOT.PBP` em
  `ms0:/PSP/GAME/TurboDash/`.
- **Sem PSP:** abra o `EBOOT.PBP` no emulador **PPSSPP**.

### Controles

| Botão             | Ação                        |
|-------------------|-----------------------------|
| D-Pad / Analógico | Mover                       |
| X                 | Pular                       |
| `>` no chão       | Boost de velocidade automático |
| `W` no chão       | Mola: pulo bem mais alto    |
| START             | Reiniciar                   |
| HOME              | Sair do jogo                |

## Objetivo

Colete gemas (`*`), derrote inimigos pisando neles, desvie dos buracos
(pulando ou usando as molas) e alcance a bandeira (`P`) com pelo menos
1 vida restante.

## Próximos passos (ideias)

- Loops visuais e câmera com auto-scroll pra sensação de velocidade real.
- Mais fases e um contador de tempo (estilo speedrun).
- Trocar o modo texto por sprites gráficos (GU), já com a base estável.
