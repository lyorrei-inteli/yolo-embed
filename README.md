# YOLO Model Server and ESP32-CAM Integration

This project includes two components:
1. A FastAPI server that runs a YOLO model for object detection on uploaded images.
2. An ESP32-CAM that captures images and sends them to the server for processing.

## Project Structure

```
/project-root
│
├── model-server                 # YOLO model server (FastAPI)
│   ├── src
│   │   ├── __init__.py
│   │   └── main.py              # FastAPI server
│   │   └── model.pt             # YOLO model file
│   ├── Dockerfile               # Dockerfile for the FastAPI server
│   ├── requirements.txt         # Python dependencies
│   └── .dockerignore            # Docker ignore file
│
├── esp-cam                      # Embedded code for ESP32-CAM (PlatformIO)
│   ├── src
│   │   └── main.cpp             # ESP32-CAM source code (image capture and POST request)
│   ├── platformio.ini           # PlatformIO configuration file
│   └── include                  # Additional header files
│
├── docker-compose.yml           # Docker Compose configuration
├── Justfile                     # Justfile for managing Docker containers easily
└── README.md                    # Project documentation
```

## Running the Model Server

Once you have cloned the repository, follow these steps to manage and run the server:

### Using Justfile

This project includes a **Justfile** to simplify Docker management commands. Below are the Justfile tasks and what they do:

1. **Bringing up the containers**:
   - This task builds and starts the Docker containers in the background.
   
   ```bash
   just up
   ```
   Equivalent to:
   ```bash
   docker-compose up --build -d
   ```

2. **Stopping the containers**:
   - This task stops all running Docker containers.
   
   ```bash
   just down
   ```
   Equivalent to:
   ```bash
   docker-compose down
   ```

3. **Restarting the containers**:
   - This task stops and restarts the containers, rebuilding if necessary.
   
   ```bash
   just restart
   ```
   Equivalent to:
   ```bash
   docker-compose down && docker-compose up --build -d
   ```

4. **Viewing the logs**:
   - This task shows real-time logs from the running containers.
   
   ```bash
   just logs
   ```
   Equivalent to:
   ```bash
   docker-compose logs -f
   ```

5. **Accessing the container shell**:
   - You can open a shell in a specific container (default is `model-server`).
   
   ```bash
   just shell
   ```
   This opens a shell in the `model-server` container, but you can specify another container:
   ```bash
   just shell container=<container_name>
   ```

6. **Checking container status**:
   - This task checks the current status of the containers.
   
   ```bash
   just status
   ```
   Equivalent to:
   ```bash
   docker-compose ps
   ```

7. **Cleaning up containers, volumes, and networks**:
   - This task removes all containers, volumes, and networks associated with the project, including images.
   
   ```bash
   just clean
   ```
   Equivalent to:
   ```bash
   docker-compose down -v --rmi all --remove-orphans
   ```

### Modifying WiFi Credentials and API Endpoint for ESP32-CAM

Before uploading the code to your ESP32-CAM, you need to change the WiFi credentials and the YOLO model server's IP address in the `main.cpp` file.

1. **Set your WiFi credentials**:
   - In the `main.cpp` file, update the `ssid` and `password` variables to match your network:
   
   ```cpp
   const char* ssid = "YourNetworkSSID";
   const char* password = "YourNetworkPassword";
   ```

2. **Update the API endpoint**:
   - Modify the `apiEndpoint` variable to the IP address where your YOLO model server is running. This address should match the server's local network IP:
   
   ```cpp
   String apiEndpoint = "http://<YOUR_SERVER_IP>:8000/run-model";
   ```

### Uploading Code to ESP32-CAM

To upload the code to your ESP32-CAM, follow these steps:

1. **Connect the ESP32-CAM to your computer** using a USB-to-serial adapter.
2. **Open PlatformIO** in your IDE (VSCode or other).
3. **Select the `esp32cam` environment** from the available environments.
4. Click on **Upload** to flash the code to the ESP32-CAM.

Alternatively, you can use the command line:

```bash
platformio run --target upload
```

### Running the ESP32-CAM

Once the code is uploaded:
1. **Power up the ESP32-CAM**.
2. It will automatically connect to the specified WiFi network.
3. The camera will capture an image every 5 seconds (or based on your configuration) and send it to the YOLO model server running at the specified IP address.

### Troubleshooting

- If the ESP32-CAM doesn't connect to WiFi, double-check the SSID and password in `main.cpp`.
- Make sure the YOLO model server is running and accessible at the specified IP and port (e.g., `http://192.168.99.135:8000/run-model`).
- Use PlatformIO’s Serial Monitor to debug issues:

```bash
platformio device monitor
```

### API Endpoints

The FastAPI server provides the following endpoints to interact with the YOLO model.

1. **`GET /`**
   - This is a simple endpoint that returns a welcome message and instructions for using the YOLO model server.

   **Response:**
   ```json
   {
     "message": "Send an image to the /run-model endpoint"
   }
   ```

2. **`POST /run-model`**
   - This is the primary endpoint that accepts an image, runs the YOLO model on it, and returns an image annotated with detected objects and bounding boxes.

   **Request:**
   - You need to send an image (in formats such as JPEG, PNG, etc.) to this endpoint using a `POST` request. The image is processed, and the result is returned as an annotated image with detected objects.

   **Response:**
   - An annotated image (JPEG) with bounding boxes marking detected objects.

   **Example:**
   ```bash
   curl -X POST "http://<YOUR_SERVER_IP>:8000/run-model" \
   -H "accept: image/jpeg" \
   -H "Content-Type: image/jpeg" \
   --data-binary @your-image-file.jpg
   ```

### Saving Input and Output Images

The FastAPI server saves both the **input image** (the image sent from the ESP32-CAM) and the **output image** (the image annotated by the YOLO model with detection results). The images are saved in the `saved_images` directory.

1. **Input Image**:
   - The raw image captured by the ESP32-CAM and sent to the server is saved with a timestamp in the filename.
   - The input image is stored in the following path:
     ```
     /model-server/saved_images/input_image_<timestamp>.jpg
     ```

2. **Output Image**:
   - After the YOLO model processes the input image and annotates it with bounding boxes around detected objects, the annotated image is saved with a timestamp.
   - The output image is stored in the following path:
     ```
     /model-server/saved_images/output_image_<timestamp>.jpg
     ```

### Example

If an image is received at `12:34:56` on September 28, 2024, both images will be saved as:

- Input image: `input_image_20240928_123456.jpg`
- Output image: `output_image_20240928_123456.jpg`

These images can be found in the `saved_images` directory inside the `model-server` folder.

### Key Additions:
1. **Justfile Documentation**:
   - Detailed documentation for each task in the Justfile, explaining what it does and how to use it.
   - Commands for container management (start, stop, restart, logs, status, clean).
   
2. **Instructions for Modifying `main.cpp`**:
   - Clear instructions for the user to change WiFi credentials and the YOLO model server IP address in the `main.cpp` file.
   - 

### Demonstration
https://github.com/user-attachments/assets/b77da9b5-2984-4c97-9909-3a2f788c56b2

