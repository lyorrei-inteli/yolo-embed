### `README.md`

# YOLO Model Server

This project sets up a FastAPI server that runs a YOLO (You Only Look Once) model for object detection on uploaded images. The server processes images by running the YOLO model, annotates them with bounding boxes, and returns the resulting image.

## Project Structure

```
/project-root
│
├── model-server
│   ├── src
│   │   ├── __init__.py
│   │   └── main.py   # FastAPI server
│   │   └── model.pt      # YOLO model file
│   ├── Dockerfile    # Dockerfile for the FastAPI server
│   ├── requirements.txt # Python dependencies
│   └── .dockerignore # Docker ignore file
├── docker-compose.yml # Docker Compose configuration
└── Justfile           # Justfile for managing Docker containers easily
```

## Running the Model Server

Once you have cloned the repository, follow these steps to manage and run the server:

### 1. Bring up the containers

You can easily bring up the containers by running:

```bash
just up
```

This command will build the Docker image (if necessary) and start the FastAPI server on port `8000`.

### 2. Stop the containers

To stop the containers, run:

```bash
just down
```

This will gracefully stop and remove the containers.

### 3. Restart the containers

To restart the containers (rebuilding if needed), use:

```bash
just restart
```

### 4. Check logs

To follow the logs from the running containers:

```bash
just logs
```

### 5. Access the shell of the container

If you need to access a shell inside the running `app` container:

```bash
just shell
```

### 6. Check the status of the containers

To verify the status of your containers:

```bash
just status
```

### 7. Clean up the environment

To remove all containers, images, volumes, and networks related to this project:

```bash
just clean
```

## API Endpoints

### `GET /`

Returns a welcome message and instructions for submitting an image.

#### Response

```json
{
  "message": "Send an image to the /run-model endpoint"
}
```

### `POST /run-model`

Accepts an image file and runs the YOLO model on it. The response is the image annotated with detection results.

#### Request

- File: An image (JPEG, PNG, etc.)

#### Response

- An image with detected objects and bounding boxes.

## Docker Compose

The project includes a `docker-compose.yml` file to simplify container management. The service defined is:

- **model-server**: The FastAPI server running the YOLO model.

Docker Compose will manage this service and ensure that it is running on port `8000`.

You can manage this service using the `just` tasks as described in the sections above.