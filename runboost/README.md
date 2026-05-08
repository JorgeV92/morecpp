# Boost Chat

Small TCP chat project built around Boost.Asio's networking model:

- asynchronous TCP accept, connect, read, and write
- `io_context` worker threads
- strands for serialized session and room handlers
- `signal_set` for Ctrl-C shutdown
- Boost.Program_options for command-line flags
- Boost.UUID guest names and Boost.Algorithm line cleanup

The design follows Boost's networking guidance for composing async operations from accept, resolve, connect, read, and write handlers.

## Build

From the repository root:

```bash
cmake -S runboost -B runboost/build
cmake --build runboost/build
```

If CMake cannot find Homebrew Boost automatically on macOS, pass the Boost prefix:

```bash
cmake -S runboost -B runboost/build -DBOOST_ROOT=/opt/homebrew/opt/boost
cmake --build runboost/build
```

## Run Locally

Start the server:

```bash
./runboost/build/chat_server --address 127.0.0.1 --port 12345
```

Open one or more client terminals:

```bash
./runboost/build/chat_client --host 127.0.0.1 --port 12345 --name alice
./runboost/build/chat_client --host 127.0.0.1 --port 12345 --name bob
```

Type chat messages in either client. Use `/name new-name` to rename yourself and `/quit` to leave.

## Useful Flags

```bash
./runboost/build/chat_server --help
./runboost/build/chat_client --help
```
