FROM gcc:13

WORKDIR /app

COPY . /app

RUN g++ -std=c++17 -O2 -Wall -Wextra -o web_server main.cpp

EXPOSE 8080

CMD ["./web_server"]
