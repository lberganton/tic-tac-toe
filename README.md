# Tic-Tac-Toe

A Tic-Tac-Toe game written in C with the Sockets POSIX API.

# Dependencies

- make
- gcc

# Build

Just run the makefile:

```sh
make
```

# How to play

## Initializing the game

You can either host a game room or connect to an existing one:

```sh
tic-tac-toe <create/connect>
```

You can specify a port to create or connect with `-p port`.
The `port` can be any value between 1024 and 65535.

If no explicit port was provided, the default one is 7878.

## Playing the game

The player one (the host) is the first to play, so, the second
player (the client) must wait for the first move, after that,
it's time for player two to play, so the player one must wait, and so on.

The plays are made by choosing a position between 1 and 9:

```
1 | 2 | 3
---+---+---
4 | 5 | 6
---+---+---
7 | 8 | 9
```

The game can finish with one of the players winning, or with
a draw if a victory cannot be determined.
