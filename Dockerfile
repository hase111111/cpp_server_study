FROM gcc:13

WORKDIR /app

COPY . /app

RUN g++ -std=c++17 -O2 -Wall -Wextra -o web_server main.cpp

ENV HOST=0.0.0.0
ENV PORT=8080

EXPOSE 8080

CMD ["./web_server"]
