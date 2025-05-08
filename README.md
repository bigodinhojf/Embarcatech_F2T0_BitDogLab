<div align="center">
    <img src="https://moodle.embarcatech.cepedi.org.br/pluginfile.php/1/theme_moove/logo/1733422525/Group%20658.png" alt="Logo Embarcatech" height="100">
</div>

<br>

# Revisão BitDogLab

## Sumário

- [Descrição](#descrição)
- [Funcionalidades Implementadas](#funcionalidades-implementadas)
- [Ferramentas utilizadas](#ferramentas-utilizadas)
- [Objetivos](#objetivos)
- [Instruções de uso](#instruções-de-uso)
- [Vídeo de apresentação](#vídeo-de-apresentação)
- [Aluno e desenvolvedor do projeto](#aluno-e-desenvolvedor-do-projeto)
- [Licensa](#licença)

## Descrição

Este projeto implementa um sistema interativo utilizando a placa de desenvolvimento BitDogLab e o microcontrolador RP2040, integrando botões, display OLED, LED RGB, matriz de LEDs RGB, joystick, buzzers e UART. O sistema tem como objetivo principal a revisão da implementação dos periféricos da BitDogLab.
O sistema possui o controle de um quadrado 8x8 no display OLED, movimentado pelo joystick, o botão do joystick altera o preenchimento do quadrado. Além disso o botão A altera a cor do LED RGB e cada vez que os botões são pressionados os buzzers emitem um sinal sonoro. Por fim, o monitor serial exibe informações dos valores ADC e da posição X e Y do quadrado, e por comunicação UART ao receber os números de 0 a 9 através do monitor serial, o dígito é exibido na matriz de LEDs.


## Funcionalidades Implementadas

1. Joystick e display OLED:

   - Os valores do ADC do joystick são lidos constantemente e são convertidos para a escala do display através das fórmulas:

<div align="center">
    <img width = "60%" src = "https://github.com/user-attachments/assets/6c302e2d-3e21-4a21-b233-f3b4831ac35e">
</div>

   - Esses valores de X e Y são a posição de um quadrado 8x8 desenhado no display OLED e movimentado pelo joystick.
   - O botão do Joystick modifica o preenchimento do quadrado 8x8 entre vazio e preenchido.

2. Botão A e LED RGB:

   - O botão A modifica a cor do LED RGB para todas as combinações simples entre os LEDs verde, azul e vermelho, as cores e ordem está descrita na imagem:

<div align="center">
    <img width = "60%" src = "https://github.com/user-attachments/assets/3ca0ae12-c453-42e5-82d7-f313763131cd">
</div>

3. Buzzers:

   - Um sinal sonoro curto de 1000 Hz é emitido sempre que qualquer botão for pressionado.

4. Monitor serial e matriz de LEDs:

   - É exibido no monitor serial as informações lidas pelo ADC e os valores x e y da posição do quadrado 8x8.
   - Ao enviar os números de 0 a 9 os dígitos são exibidos na matriz de LEDs RGB na cor ciano.
  
## Ferramentas utilizadas

- **Ferramenta educacional BitDogLab (versão 6.3)**: Placa de desenvolvimento utilizada para programar o microcontrolador.
- **Microcontrolador Raspberry Pi Pico W**: Responsável pela leitura do ADC, controle dos periféricos e comunicação serial via UART.
- **Joystick com botão**: Utilizado para mover um quadrado 8x8 no display OLED e alterar seu preenchimento.
- **Display OLED SSD1306**: Exibe o quadrado 8x8.
- **Botão A**: Utilizado para alterar a cor do LED RGB.
- **LED RGB**: Muda a cor de acordo com o botão A.
- **Buzzers**: Toca os sinais sonoros sempre que um botão é pressionado.
- **Matriz de LEDs RGB**: Exibe o dígito enviado pelo monitor serial.
- **Monitor serial**: Exibe informações do ADC e recebe comandos via UART.
- **Visual Studio Code (VS Code)**: IDE utilizada para o desenvolvimento do código com integração ao Pico SDK.
- **Pico SDK**: Kit de desenvolvimento de software utilizado para programar o Raspberry Pi Pico W em linguagem C.

## Objetivos

1. Revisar os conceitos estudados na fase 1 de capacitação do EmbarcaTech.
2. Implementar um sistema integrado utilizando os periféricos da BitDogLab.

## Instruções de uso

1. **Clonar o Repositório**:

```bash
git clone https://github.com/bigodinhojf/Embarcatech_F2T0_BitDogLab.git
```

2. **Compilar e Carregar o Código**:
   No VS Code, configure o ambiente e compile o projeto com os comandos:

```bash	
cmake -G Ninja ..
ninja
```

3. **Interação com o Sistema**:
   - Conecte a placa ao computador.
   - Clique em run usando a extensão do raspberry pi pico.
   - Movimente o joystick e observe a movimentação no display OLED.
   - Pressione o botão do joystick para alterar o preenchimento do quadrado 8x8.
   - Pressione o botão A e observe a alteração das cores do LED RGB.
   - Ao pressionar qualquer botão, observe o sinal sonoro emitido pelos buzzers.
   - Abra o monitor serial e observe as informações exibidas.
   - Envie os números de 0 a 9 e observe a exibição dos dígitos na matriz de LEDs.

## Vídeo de apresentação

O vídeo apresentando o projeto pode ser assistido [clicando aqui](https://youtu.be/sReq7fx1kIk).

## Aluno e desenvolvedor do projeto

<a href="https://github.com/bigodinhojf">
        <img src="https://github.com/bigodinhojf.png" width="150px;" alt="João Felipe"/><br>
        <sub>
          <b>João Felipe</b>
        </sub>
</a>

## Licença

Este projeto está licenciado sob a licença MIT.
