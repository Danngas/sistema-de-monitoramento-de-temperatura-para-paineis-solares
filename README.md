# Sistema de Monitoramento de Temperatura para Painéis Solares
Daniel Silva de souza

link do Video: https://youtu.be/9FZJZ7f-6wM?si=kijlpPfXn_Z4Mnjq

Um sistema embarcado baseado no Raspberry Pi Pico para monitoramento em tempo real de temperatura em painéis solares, com interface gráfica em display OLED e sistema de alertas visuais.

## 📋 Características

- Display OLED SSD1306 128x64
- Sensor de temperatura analógico
- Indicador LED RGB para alertas
- Interface com 2 botões de navegação
- Múltiplas telas de visualização
- Sistema de alertas configurável
- Armazenamento de histórico
- Visualização em tempo real
- Estatísticas de temperatura

## 🔧 Hardware Necessário

- Raspberry Pi Pico
- Display OLED SSD1306 (128x64)
- Sensor de temperatura analógico
- LED RGB
- 2 botões push-button
- Resistores pull-up para botões
- Cabos de conexão

### Pinagem

- **I2C (Display OLED)**
  - SDA: GPIO 14
  - SCL: GPIO 15

- **Botões**
  - Botão A: GPIO 5
  - Botão B: GPIO 6

- **LED RGB**
  - Vermelho: GPIO 13
  - Verde: GPIO 11
  - Azul: GPIO 12

- **Sensor**
  - Temperatura: GPIO 26 (ADC0)

## 🚀 Funcionalidades

### 1. Tela Inicial (Splash Screen)
- Logo do sistema
- Versão do software
- Instruções básicas de navegação

### 2. Menu Principal
Navegue entre as opções usando o Botão A, selecione com o Botão B:
- Monitor
- Histórico
- Configuração
- Estatísticas
- Alertas

### 3. Monitor em Tempo Real
- Gráfico de temperatura em tempo real
- Valor atual da temperatura
- Escala automática
- Atualização contínua

### 4. Histórico
- Visualização das últimas 128 leituras
- Rolagem através do histórico
- Indicadores de navegação
- Timestamps das medições

### 5. Configuração
Ajuste os parâmetros do sistema:
- Limite de temperatura normal
- Limite de atenção
- Limite de urgência
- Intervalo de amostragem

### 6. Estatísticas
Exibe dados estatísticos:
- Temperatura atual
- Temperatura máxima
- Temperatura mínima
- Média de temperatura

### 7. Sistema de Alertas
Três níveis de alerta com indicação visual por LED RGB:
- **Normal (Verde)**: Operação normal
- **Atenção (Amarelo)**: Temperatura elevada
- **Urgente (Vermelho Piscante)**: Temperatura crítica

## 🎮 Navegação

- **Botão A**: 
  - No menu: Navega entre as opções
  - Nas telas: Retorna ao menu

- **Botão B**:
  - No menu: Seleciona a opção
  - Nas telas: Função específica da tela

## 🛠️ Instalação

1. Clone o repositório:
   ```bash
   git clone https://github.com/seu-usuario/sistema-temperatura.git
   ```

2. Configure o ambiente de desenvolvimento Pico SDK

3. Compile o projeto:
   ```bash
   pico-sdk build
   ```


4. Carregue o arquivo .uf2 gerado no Raspberry Pi Pico

## 📦 Estrutura do Projeto

├── lib/
│ ├── ssd1306.h
│ ├── ssd1306.c
│ └── font.h
├── CMakeLists.txt
├── System_Monitor_Temp_PV.c
└── README.md


## 🔄 Ciclo de Amostragem

- Intervalo de amostragem: 1000ms
- Buffer circular de 128 amostras
- Atualização do display: 50ms
- Debounce dos botões: 200ms

## 🔧 Customização

O sistema pode ser customizado através das definições no início do arquivo principal:
- Limites de temperatura
- Intervalos de amostragem
- Tamanho do buffer
- Pinagem
- Configurações do display

## 📈 Melhorias Futuras

- [ ] Armazenamento em memória não-volátil
- [ ] Comunicação wireless
- [ ] Interface web
- [ ] Mais opções de sensores
- [ ] Exportação de dados

## 📄 Licença

Este projeto está sob a licença MIT - veja o arquivo [LICENSE.md](LICENSE.md) para detalhes

## ✨ Contribuições

Contribuições são bem-vindas! Por favor, leia o [CONTRIBUTING.md](CONTRIBUTING.md) para detalhes sobre nosso código de conduta e o processo para enviar pull requests.

## 🤝 Suporte

Se você tiver alguma dúvida ou sugestão, por favor abra uma issue no repositório do projeto.
