FROM python:3.10-slim
WORKDIR /app
COPY standalone_server/ /app/
COPY HTML_data/ /HTML_data/
RUN pip install --no-cache-dir -r requirements.txt
EXPOSE 5001
CMD ["python", "app_fastapi.py"]