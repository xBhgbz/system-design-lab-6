FROM ubuntu:22.04

# Установка зависимостей и инструментов сборки
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libpoco-dev \
    librabbitmq-dev \
    libboost-all-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Клонирование, сборка и установка SimpleAmqpClient
RUN git clone https://github.com/alanxz/SimpleAmqpClient.git /tmp/SimpleAmqpClient && \
    cd /tmp/SimpleAmqpClient && \
    mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc) && \
    make install && \
    ldconfig && \
    rm -rf /tmp/SimpleAmqpClient

# Рабочая директория
WORKDIR /app

# Копирование файлов проекта
COPY CMakeLists.txt .
COPY src ./src
COPY include ./include

# Создание директории сборки и компиляция
RUN mkdir -p build && \
    cd build && \
    cmake .. && \
    make -j$(nproc)

# Работаем из директории build
WORKDIR /app/build

EXPOSE 8080

# По умолчанию запускаем API сервер
CMD ["./api"]