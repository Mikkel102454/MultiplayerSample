# MultiplayerSample — Project Notes (Head Start)

## Purpose
**MultiplayerSample** is a **game prototype** written in **C++20** using **raylib** for windowing/rendering. The game includes an in-game **developer console** that’s used as the primary interface to host/join/inspect multiplayer behavior.

Networking is **TCP-focused right now**. UDP may be considered later, but there are **no current plans** to implement UDP gameplay networking.

## Platform / Toolchain
- OS target: **Windows**
- Toolchain used: **MinGW**
- Build system: **CMake**
- Network backend: **WinSock2** (`ws2_32`)

## Dependencies (from CMake)
Fetched via CMake `FetchContent`:
- **raylib** (pulled from the upstream repo, tracking the `master` tag/branch)
- **Lua** (via `lua-cmake`, tag `lua-cmake-5.4`)

Linked libraries:
- `raylib`
- `lua_library`
- `ws2_32`

## Build configuration notes
Compile definitions observed:
- Debug builds define:
    - `ASSETS_PATH="<repo>/assets/"`
- Always defines:
    - `GAME_VERSION="1.0.0"`
    - `MEMORY_RUNTIME_SAFETY=1`
- Platform defines:
    - `PLATFORM_WINDOWS` on Windows

## Repo Layout (high-level)
- `assets/` — runtime assets (path injected in Debug via `ASSETS_PATH`)
- `include/` — headers:
    - `input/` — input system (devices, contexts, keybinds/actions)
    - `manager/` — global-ish managers (Console/Client/Server)
    - `network/` — client/server implementation + packet definitions
    - `util/` — net wrappers, drawing helpers, numbers, dev console implementation
    - `player.h`
- `src/` — implementations mirroring `include/` structure
- Build folders exist in-repo (e.g. `cmake-build-debug-mingw`, `cmake-build-debug-visual-studio`)

## Deployment / Hosting Model
- **Host player** runs `start_server ...`:
    - Their game instance runs **a server** (and also runs the game normally).
    - “One server per hosted game instance.”
- **Joiner player** runs `join_server ...`:
    - Their game instance runs **client only**.
    - They do **not** host a server.
- Dedicated headless server is **not** the current model.

## How to run / control (In-game Console)
Multiplayer is controlled through the in-game console.

### Console commands
- `start_server {ip} {port}`  
  Start a server bound to `{ip}:{port}`.

- `stop_server`  
  Stop the active server (if any).

- `join_server {ip} {port} {username}`  
  Join a server at `{ip}:{port}` using `{username}` as the player name.

- `quit_server`  
  Quit/leave the server you are currently connected to.

- `list`  
  List all users in the current server (player list).

### Defaults / conventions
- **No default port**: `{port}` is always provided explicitly.
- Common local testing values:
    - `{ip}`: `127.0.0.1` (same machine testing)
    - `{port}`: any free port you choose

## Runtime Overview
- Raylib window created; game loop runs at target FPS.
- Networking is initialized at startup and shut down at exit.
- A global console is created early and can be toggled during runtime.
- Client networking (if a client exists) is updated from the main loop.
- On shutdown: server is stopped (if running), client is disconnected (if connected), then networking + console are shut down.

## Manager Pattern (Global-ish singletons)
Several systems are managed through static manager classes that internally hold an `std::optional<T>` instance and expose:
- `create()` (construct if missing)
- `has()`
- `get()`
- `destroy()/leave()/stop()` (reset)

Managers present:
- `ConsoleManager`
- `ClientManager`
- `ServerManager`

## Networking Overview (Current)
### Source of truth for protocol
- `network/packets.*` is the **canonical definition** of packet types and payload layouts.

### High-level structure
- A `Net` / `Socket` abstraction wraps WinSock2.
- TCP sockets are used for the current client/server connection model.
- A polling mechanism (select-based) is used to check socket readability/writability.

### Player identity / IDs
- Players connect with a **username**.
- The **server assigns each player an integer ID** (`int`).

### Player list flow
- `list` requests a list of connected players.
- Server responds with a header (count) plus individual player entries.

## Known Risks / “Watch-outs” (observed patterns to remember)
These are not action items right now—just notes to remember when debugging later:

1) **Client ID vs container index**
    - Server-assigned `id` values are stable identifiers.
    - If connected clients are stored in a vector and elements are erased, indices shift; code must avoid treating `id` as a direct vector index unless explicitly guaranteed.

2) **TCP stream framing**
    - TCP delivers a byte stream; receiving a fixed-size payload may require multiple reads in real conditions.
    - Packet parsing must be careful to avoid desync if reads are partial.

3) **Server thread lifetime**
    - If server logic runs on a separate thread, shutdown and object lifetime must be coordinated carefully to avoid accessing destroyed objects.

## Non-goals (for now)
- UDP gameplay networking is **not planned right now**.

---
Last updated: 2026-02-08