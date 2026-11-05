FROM gcc:14-bookworm AS build
WORKDIR /workspace

RUN apt-get update \
    && apt-get install -y --no-install-recommends cmake git \
    && rm -rf /var/lib/apt/lists/*

COPY CMakeLists.txt ./
COPY src/ ./src/
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel

FROM debian:bookworm-slim
LABEL org.opencontainers.image.title="Agenda de contacte" \
      org.opencontainers.image.authors="Lăzărescu Alexandru-Marcel <alexandru-marcel.lazarescu@student.upt.ro>"

ARG COMMIT=dev
ARG BUILT_AT=unknown
ENV APP_COMMIT=${COMMIT} APP_BUILT_AT=${BUILT_AT}

RUN useradd --uid 10001 --create-home app
COPY --from=build /workspace/build/server /usr/local/bin/server
USER app
EXPOSE 8080
ENTRYPOINT ["/usr/local/bin/server"]
