from fastapi import FastAPI, File, UploadFile
from fastapi.responses import StreamingResponse
from ultralytics import YOLO

import cv2
import numpy as np
from io import BytesIO

app = FastAPI()

model = YOLO("./src/model.pt")


@app.post("/run-model")
async def run_model(file: UploadFile = File(...)):
    image_bytes = await file.read()
    np_img = np.frombuffer(image_bytes, np.uint8)
    img = cv2.imdecode(np_img, cv2.IMREAD_COLOR)

    # Run YOLO model inference
    results = model(img)

    # Annotate the image with the detection results (bounding boxes, etc.)
    annotated_img = results[0].plot()

    # Convert the annotated image to JPEG
    _, img_encoded = cv2.imencode(".jpg", annotated_img)
    img_bytes = BytesIO(img_encoded.tobytes())

    # Return the image as a streaming response
    return StreamingResponse(img_bytes, media_type="image/jpeg")


@app.get("/")
def read_root():
    return {"message": "Send an image to the /run-model endpoint"}
