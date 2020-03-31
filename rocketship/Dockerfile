FROM buildpack-deps:buster

ENV RUSTUP_HOME=/usr/local/rustup \
    CARGO_HOME=/usr/local/cargo \
    PATH=/usr/local/cargo/bin:$PATH

RUN set -eux; \
    \
    url="https://static.rust-lang.org/rustup/dist/x86_64-unknown-linux-gnu/rustup-init"; \
    wget "$url"; \
    chmod +x rustup-init; \
    ./rustup-init -y --no-modify-path --default-toolchain nightly; \
    rm rustup-init; \
    chmod -R a+w $RUSTUP_HOME $CARGO_HOME; \
    rustup --version; \
    cargo --version; \
    rustc --version;

WORKDIR /app
COPY ./ /app

RUN cargo build --release --bin server \
    && cargo build --bin show_measurement \
    && cargo build --bin write_measurement \
    && cargo build --bin delete_measurement

# I'm running these manually in the container for now...
# some weird db connection error is causing trouble
# cargo install diesel_cli --no-default-features --features postgres

#RUN cargo install diesel_cli
#RUN diesel setup

ENTRYPOINT ["/app/target/release/server"]
