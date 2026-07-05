FROM gcc:13

RUN apt-get update \
    && apt-get install -y cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . /app

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build

ENV HOST=0.0.0.0
ENV PORT=8080

EXPOSE 8080

CMD ["./build/web_server"]
