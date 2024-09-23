from fastapi import FastAPI, File, UploadFile
from fastapi.responses import StreamingResponse
from ultralytics import YOLO

import cv2
import numpy as np

app = FastAPI()

model = YOLO("./src/model.pt")


@app.post("/run-model")
async def run_model(file: UploadFile = File(...)):
    image_bytes = await file.read()
    np_img = np.frombuffer(image_bytes, np.uint8)
    img = cv2.imdecode(np_img, cv2.IMREAD_COLOR)

    results = model(img)
    return StreamingResponse(results[0], media_type="image/jpeg")


@app.get("/")
def read_root():
    return {"message": "Send an image to the /run-model endpoint"}
