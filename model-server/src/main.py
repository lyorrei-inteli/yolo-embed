from fastapi import FastAPI, Request
from fastapi.responses import FileResponse
from ultralytics import YOLO
import cv2
import numpy as np
from io import BytesIO
import os
from datetime import datetime

app = FastAPI()

model = YOLO("./src/model.pt")  # Load your YOLO model

# Create directory to save images if it doesn't exist
os.makedirs("saved_images", exist_ok=True)

def save_image(image, filename):
    """Utility function to save an image to disk."""
    filepath = os.path.join("saved_images", filename)
    cv2.imwrite(filepath, image)
    return filepath

@app.post("/run-model")
async def run_model(request: Request):
    # Read the raw binary image data from the request body
    image_bytes = await request.body()

    # Convert the raw bytes into a NumPy array
    np_img = np.frombuffer(image_bytes, np.uint8)

    # Decode the NumPy array into an OpenCV image
    img = cv2.imdecode(np_img, cv2.IMREAD_COLOR)

    # Save the incoming image with a timestamped filename
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    input_image_path = save_image(img, f"input_image_{timestamp}.jpg")

    # Run YOLO model inference
    results = model(img)

    # Annotate the image with the detection results (bounding boxes, etc.)
    annotated_img = results[0].plot()

    # Save the output image (annotated) with a timestamped filename
    output_image_path = save_image(annotated_img, f"output_image_{timestamp}.jpg")

    # Convert the annotated image to JPEG
    _, img_encoded = cv2.imencode(".jpg", annotated_img)
    img_bytes = BytesIO(img_encoded.tobytes())

    # Log the saved paths
    print(f"Input image saved at: {input_image_path}")
    print(f"Output image saved at: {output_image_path}")

    # Return the annotated image as a streaming response
    return FileResponse(output_image_path, media_type="image/jpeg")

@app.get("/")
def read_root():
    return {"message": "Send an image to the /run-model endpoint"}
