FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libpqxx-dev \
    libpq-dev \
    libssl-dev \
    wget \
    ca-certificates \
    postgresql-client \
    git \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# grab single-header httplib for convenience
RUN mkdir -p include && \
    wget -q -O include/httplib.h https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h

COPY . /app

RUN mkdir -p build && cd build && cmake .. && make -j$(nproc)

COPY docker/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

EXPOSE 8080
CMD ["/entrypoint.sh"]
