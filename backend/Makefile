VERSION = 0.1.9

all: build

build:
	cargo build --release
	cp ./target/release/backend ../build/ats3-backend

debug:
	cargo build
	cp ./target/debug/backend ../build/ats3-backend
