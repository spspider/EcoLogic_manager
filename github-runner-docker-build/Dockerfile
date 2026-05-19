FROM ubuntu:20.04

# Установка зависимостей
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    curl \
    python3 \
    python3-pip \
    python3-serial \
    git \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# Установка arduino-cli
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
RUN mv bin/arduino-cli /usr/local/bin/

# Инициализация arduino-cli и установка ESP8266
RUN arduino-cli config init && \
    arduino-cli config add board_manager.additional_urls http://arduino.esp8266.com/stable/package_esp8266com_index.json && \
    arduino-cli core update-index && \
    arduino-cli core install esp8266:esp8266@3.1.2

# Исправляем python3 для ESP8266
RUN rm -f /root/.arduino15/packages/esp8266/tools/python3/3.7.2-post1/python3 && \
    cp /usr/bin/python3 /root/.arduino15/packages/esp8266/tools/python3/3.7.2-post1/python3 && \
    chmod +x /root/.arduino15/packages/esp8266/tools/python3/3.7.2-post1/python3

WORKDIR /workspace

CMD ["sh"]

#docker build -t spspider/esp8266-clean:3.1.2 .
#docker tag esp8266-clean spspider/esp8266-clean:3.1.2
# docker push spspider/esp8266-clean:3.1.2