# YOLO Model Server and ESP32-CAM Integration with RTOS and Web Server

Este projeto integra dois componentes principais:
1. Um servidor FastAPI que executa um modelo YOLO para detecção de objetos em imagens enviadas.
2. Um ESP32-CAM que captura imagens, as envia ao servidor para processamento e exibe a imagem processada em uma interface web.

O ESP32-CAM utiliza FreeRTOS para gerenciar tarefas concorrentes, garantindo que as operações de captura de imagem, envio ao servidor e visualização via web ocorram de forma eficiente.

## Estrutura do Projeto

```
/project-root
│
├── model-server                 # Servidor do modelo YOLO (FastAPI)
│   ├── src
│   │   ├── __init__.py
│   │   └── main.py              # Servidor FastAPI
│   │   └── model.pt             # Arquivo do modelo YOLO
│   ├── Dockerfile               # Dockerfile para o servidor FastAPI
│   ├── requirements.txt         # Dependências Python
│   └── .dockerignore            # Arquivo de ignorância do Docker
│
├── esp-cam                      # Código embarcado para ESP32-CAM (PlatformIO)
│   ├── src
│   │   └── main.cpp             # Código fonte do ESP32-CAM (RTOS e servidor web)
│   ├── platformio.ini           # Arquivo de configuração do PlatformIO
│   └── include                  # Arquivos de cabeçalho adicionais
│
├── docker-compose.yml           # Configuração do Docker Compose
├── Justfile                     # Justfile para gerenciar containers Docker facilmente
└── README.md                    # Documentação do projeto
```

## Integração com FreeRTOS

O ESP32-CAM utiliza o FreeRTOS para gerenciar três tarefas principais:
1. **Tarefa de Aquisição de Imagem**: Captura imagens da câmera a cada alguns segundos.
2. **Tarefa de Envio de Imagem**: Envia a imagem capturada ao servidor do modelo YOLO para processamento.
3. **Tarefa do Servidor Web**: Hospeda um servidor web leve para exibir a imagem processada.

### Visão Geral das Tarefas

- **Tarefa de Aquisição de Imagem**: Captura uma imagem do módulo da câmera e disponibiliza para processamento. Um semáforo é usado para garantir a sincronização adequada entre as tarefas.

- **Tarefa de Envio de Imagem**: Envia a imagem capturada ao servidor do modelo YOLO. Depois de processada, a imagem anotada é recebida e armazenada em um buffer para exibição.

- **Tarefa do Servidor Web**: O ESP32-CAM executa um servidor web leve que serve uma página HTML mostrando a imagem processada. Os usuários podem acessar esta página conectando-se ao endereço IP do ESP32-CAM na rede local.

## Executando o Servidor do Modelo

Depois de clonar o repositório, siga estas etapas para gerenciar e executar o servidor:

### Usando o Justfile

Este projeto inclui um **Justfile** para simplificar os comandos de gerenciamento do Docker. Abaixo estão as tarefas disponíveis no Justfile:

1. **Iniciar os containers**:
   - Esta tarefa constrói e inicia os containers Docker em segundo plano.
   
   ```bash
   just up
   ```
   Equivalente a:
   ```bash
   docker-compose up --build -d
   ```

2. **Parar os containers**:
   - Esta tarefa para todos os containers Docker em execução.
   
   ```bash
   just down
   ```
   Equivalente a:
   ```bash
   docker-compose down
   ```

3. **Reiniciar os containers**:
   - Esta tarefa para e reinicia os containers, reconstruindo-os se necessário.
   
   ```bash
   just restart
   ```
   Equivalente a:
   ```bash
   docker-compose down && docker-compose up --build -d
   ```

4. **Visualizar os logs**:
   - Esta tarefa exibe os logs em tempo real dos containers em execução.
   
   ```bash
   just logs
   ```
   Equivalente a:
   ```bash
   docker-compose logs -f
   ```

5. **Acessar o shell do container**:
   - Abre um shell em um container específico (por padrão, `model-server`).
   
   ```bash
   just shell
   ```
   Abre um shell no container `model-server`, mas você pode especificar outro container:
   ```bash
   just shell container=<container_name>
   ```

6. **Verificar o status dos containers**:
   - Esta tarefa verifica o status atual dos containers.
   
   ```bash
   just status
   ```
   Equivalente a:
   ```bash
   docker-compose ps
   ```

7. **Limpar containers, volumes e redes**:
   - Remove todos os containers, volumes e redes associados ao projeto, incluindo imagens.
   
   ```bash
   just clean
   ```
   Equivalente a:
   ```bash
   docker-compose down -v --rmi all --remove-orphans
   ```

## Modificando Credenciais de WiFi e Endpoint da API no ESP32-CAM

Antes de carregar o código para o ESP32-CAM, você precisa configurar as credenciais de WiFi e o endereço IP do servidor do modelo YOLO no arquivo `main.cpp`.

1. **Configurar suas credenciais de WiFi**:
   - No arquivo `main.cpp`, atualize as variáveis `ssid` e `password` para corresponder à sua rede:

   ```cpp
   const char* ssid = "YourNetworkSSID";
   const char* password = "YourNetworkPassword";
   ```

2. **Atualizar o endpoint da API**:
   - Modifique a variável `apiEndpoint` para o endereço IP em que o servidor do modelo YOLO está sendo executado:

   ```cpp
   String apiEndpoint = "http://<YOUR_SERVER_IP>:8000/run-model";
   ```

## Carregando o Código no ESP32-CAM

Para enviar o código para o ESP32-CAM, siga estas etapas:

1. **Conecte o ESP32-CAM ao seu computador** usando um adaptador USB-para-serial.
2. **Abra o PlatformIO** no seu IDE (VSCode ou outro).
3. **Selecione o ambiente `esp32cam`** dentre os ambientes disponíveis.
4. Clique em **Upload** para enviar o código para o ESP32-CAM.

Alternativamente, você pode usar o comando na linha de comando:

```bash
platformio run --target upload
```

## Executando o ESP32-CAM

Depois de carregar o código:
1. **Ligue o ESP32-CAM**.
2. Ele se conectará automaticamente à rede WiFi especificada.
3. A câmera capturará uma imagem a cada 5 segundos (ou conforme configurado) e enviará ao servidor do modelo YOLO no endereço IP especificado.

### Servidor Web no ESP32-CAM

O ESP32-CAM hospeda um servidor web leve que permite visualizar a imagem processada.

- Acesse a página inicial do servidor digitando o IP do ESP32-CAM no navegador (por exemplo: `http://192.168.1.100/`).
- A página exibirá a imagem processada pela YOLO, que é recebida do servidor após ser enviada pelo ESP32-CAM.

## Resolvendo Problemas

- Caso o ESP32-CAM não consiga se conectar ao WiFi, verifique o SSID e a senha no `main.cpp`.
- Certifique-se de que o servidor do modelo YOLO está em execução e acessível no IP e porta especificados (por exemplo: `http://192.168.99.135:8000/run-model`).
- Use o Monitor Serial do PlatformIO para depurar problemas:

```bash
platformio device monitor
```

## Endpoints da API

O servidor FastAPI fornece os seguintes endpoints para interação com o modelo YOLO:

1. **`GET /`**:
   - Um endpoint simples que retorna uma mensagem de boas-vindas e instruções para usar o servidor do modelo YOLO.

   **Resposta**:
   ```json
   {
     "message": "Send an image to the /run-model endpoint"
   }
   ```

2. **`POST /run-model`**:
   - Este é o endpoint principal que aceita uma imagem, executa o modelo YOLO nela e retorna uma imagem anotada com os objetos detectados.

   **Requisição**:
   - Envie uma imagem (em formatos como JPEG, PNG, etc.) para este endpoint usando um `POST`. A imagem é processada, e o resultado é retornado como uma imagem anotada.

   **Resposta**:
   - Uma imagem anotada (JPEG) com caixas delimitadoras marcando os objetos detectados.

   **Exemplo**:
   ```bash
   curl -X POST "http://<YOUR_SERVER_IP>:8000/run-model" \
   -H "accept: image/jpeg" \
   -H "Content-Type: image/jpeg" \
   --data-binary @your-image-file.jpg
   ```

## Salvando Imagens de Entrada e Sa\u00edda

O servidor FastAPI salva tanto a **imagem de entrada** (enviada pelo ESP32-CAM) quanto a **imagem de saída** (anotada pelo modelo YOLO).

1. **Imagem de Entrada**:
   - Salva com um timestamp no nome do arquivo:
     ```
     /model-server/saved_images/input_image_<timestamp>.jpg
     ```

2. **Imagem de Saída**:
   - Salva após o processamento, também com um timestamp:
     ```
     /model-server/saved_images/output_image_<timestamp>.jpg
     ```

### Exemplos

Se uma imagem for recebida às `12:34:56` em 28 de setembro de 2024, as imagens serão salvas como:

- Imagem de entrada: `input_image_20240928_123456.jpg`
- Imagem de saída: `output_image_20240928_123456.jpg`

Estas imagens podem ser encontradas no diretório `saved_images` dentro da pasta `model-server`.

### Demonstração

#### Ponderada 2
https://github.com/user-attachments/assets/b77da9b5-2984-4c97-9909-3a2f788c56b2

#### Ponderada 3

