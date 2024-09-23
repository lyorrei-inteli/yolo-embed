
FROM python:3.10
WORKDIR /app

RUN apt-get update && apt-get install -y \
  libgl1-mesa-glx \
  libglib2.0-0 \
  libsm6 \
  libxext6 \
  libxrender-dev \
  && rm -rf /var/lib/apt/lists/*

RUN pip install uv

COPY requirements.txt .

RUN uv pip install --no-cache-dir --system -r requirements.txt

COPY . .

EXPOSE 8000

CMD ["fastapi", "run", "src/main.py"]
