FROM python:3.11-alpine

RUN mkdir /app
COPY main.py /app
RUN chmod +x /app/main.py

WORKDIR /app
CMD ["python3", "main.py"]
