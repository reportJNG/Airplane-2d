# Airplane 2D

React + Vite shell for the Raylib C airplane game.

## Scripts

- `npm run dev` starts the React app.
- `npm run game:build` compiles `AirPlane Game/game.c` to WebAssembly and writes `public/game/airplane.js`, `public/game/airplane.wasm`, and `public/game/airplane.data`.
- `npm run lint` runs ESLint.
- `npm run build` creates the production build.
- `npm run preview` serves the production build locally.

`game:build` expects Emscripten `emcc` to be active. If `emcc` is not already on PATH, the script also checks `C:\Work\Tools\emsdk\emsdk_env.bat`.
