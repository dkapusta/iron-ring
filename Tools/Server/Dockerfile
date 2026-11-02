# Iron Ring dedicated server image (docs/server.md).
#
#   docker compose up -d --build                 (docker-compose.yml in the repository root)
# or by hand:
#   docker build -f Tools/Server/Dockerfile -t ironring-server .        [--build-arg BUILD_JOBS=4]
#   docker run -d --name ironring -p 7727:7727/udp --restart unless-stopped ironring-server
#
# Built from the repository root. The server needs no SDL; the game data it
# referees with is packed from the same checkout, so it always matches.

FROM debian:trixie-slim AS build
RUN apt-get update \
 && apt-get install -y --no-install-recommends cmake ninja-build g++ git ca-certificates \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /src
# Parallel compile jobs: each needs about 1 GB, so small servers build one at a time.
ARG BUILD_JOBS=1
COPY . .
RUN cmake -S . -B /build -G Ninja -DCMAKE_BUILD_TYPE=Release -DIR_BUILD_PLAYER=OFF -DIR_BUILD_TESTS=OFF \
 && cmake --build /build --target IronRingServer AssetPacker -j "${BUILD_JOBS}" \
 && mkdir -p /out \
 && /build/Tools/Release/AssetPacker --project . --pipeline ProjectSettings/AssetPipeline.json \
        --output /out/IronRing_Data \
 && cp /build/Server/Release/IronRingServer /out/ \
 && sed 's/^log=.*/log=/' Source/Server/server.cfg > /out/server.cfg

FROM debian:trixie-slim
RUN useradd --system --home-dir /srv/ironring --shell /usr/sbin/nologin ironring
COPY --from=build --chown=ironring:ironring /out/ /srv/ironring/
USER ironring
WORKDIR /srv/ironring
EXPOSE 7727/udp
# The log goes to stdout (docker logs); settings can be overridden: docker run ... ironring-server --name "MY RING"
ENTRYPOINT ["/srv/ironring/IronRingServer"]
