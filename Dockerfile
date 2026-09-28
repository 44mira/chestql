# Stage 1:
FROM archlinux:base-devel AS builder

WORKDIR /app

# get dependencies
RUN pacman -Syu --noconfirm \
  meson \
  ninja \
  git \
  cmake \
  curl \
  gcc


COPY . .

# cache and download wrap dependencies
RUN meson subprojects download

# compile project
RUN meson setup build && meson compile -C build

# Stage 2:
FROM archlinux:base

WORKDIR /app

COPY --from=builder /app/build/chestql /app/

# expose port
EXPOSE 4499 

CMD ["./build/chestql"]
