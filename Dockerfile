FROM gcc:13

WORKDIR /app

COPY . /app

RUN make

ENV HOST=0.0.0.0
ENV PORT=8080

EXPOSE 8080

CMD ["./web_server"]
