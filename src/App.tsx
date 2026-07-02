import { useCallback, useEffect, useMemo, useRef, useState } from "react";
import type { RefObject } from "react";
import "./App.css";

type GameStatus = "idle" | "loading" | "ready" | "error";

type AirplaneModule = {
  canvas?: HTMLCanvasElement;
  ccall?: (
    ident: string,
    returnType: null,
    argTypes: string[],
    args: number[],
  ) => unknown;
  locateFile?: (path: string) => string;
  onRuntimeInitialized?: () => void;
  print?: (text: string) => void;
  printErr?: (text: string) => void;
  setStatus?: (text: string) => void;
};

declare global {
  interface Window {
    Module?: AirplaneModule;
    airplaneGameScriptLoaded?: boolean;
  }
}

const keyBindings = [
  { label: "Left", detail: "ArrowLeft / A", code: 263 },
  { label: "Right", detail: "ArrowRight / D", code: 262 },
  { label: "Shoot", detail: "Space", code: 32 },
  { label: "Replay", detail: "Enter", code: 257 },
  { label: "Pause", detail: "Escape", code: 256 },
] as const;

const browserHandledKeys = new Set([
  "ArrowLeft",
  "ArrowRight",
  " ",
  "Spacebar",
  "Enter",
  "Escape",
  "a",
  "A",
  "d",
  "D",
]);

function App() {
  return <GameShell />;
}

function GameShell() {
  const gameFrameRef = useRef<HTMLDivElement>(null);
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const [status, setStatus] = useState<GameStatus>("idle");
  const [statusText, setStatusText] = useState("Click the square to start");
  const [isFullscreen, setIsFullscreen] = useState(false);

  const canSendInput = status === "ready";

  useEffect(() => {
    const syncFullscreen = () => {
      setIsFullscreen(document.fullscreenElement === gameFrameRef.current);
    };

    document.addEventListener("fullscreenchange", syncFullscreen);
    return () => document.removeEventListener("fullscreenchange", syncFullscreen);
  }, []);

  useEffect(() => {
    const stopPageScroll = (event: KeyboardEvent) => {
      if (browserHandledKeys.has(event.key)) event.preventDefault();
    };

    window.addEventListener("keydown", stopPageScroll, { passive: false });
    return () => window.removeEventListener("keydown", stopPageScroll);
  }, []);

  const sendGameKey = useCallback((code: number, isDown: boolean) => {
    window.Module?.ccall?.("SetBrowserKey", null, ["number", "number"], [
      code,
      isDown ? 1 : 0,
    ]);
  }, []);

  const startGame = useCallback(() => {
    if (status !== "idle" || !canvasRef.current) {
      canvasRef.current?.focus();
      return;
    }

    setStatus("loading");
    setStatusText("Loading game assets...");

    window.Module = {
      canvas: canvasRef.current,
      locateFile: (path: string) => `/game/${path}`,
      onRuntimeInitialized: () => {
        setStatus("ready");
        setStatusText("Game running");
        canvasRef.current?.focus();
      },
      print: (text: string) => console.log(text),
      printErr: (text: string) => console.error(text),
      setStatus: (text: string) => {
        if (text) setStatusText(text);
      },
    };

    if (window.airplaneGameScriptLoaded) {
      setStatus("ready");
      setStatusText("Game running");
      canvasRef.current.focus();
      return;
    }

    const script = document.createElement("script");
    script.src = "/game/airplane.js";
    script.async = true;
    script.onload = () => {
      window.airplaneGameScriptLoaded = true;
    };
    script.onerror = () => {
      setStatus("error");
      setStatusText("Could not load /game/airplane.js");
    };
    document.body.appendChild(script);
  }, [status]);

  const toggleFullscreen = useCallback(async () => {
    const frame = gameFrameRef.current;
    if (!frame) return;

    if (document.fullscreenElement === frame) {
      await document.exitFullscreen();
      return;
    }

    await frame.requestFullscreen();
    canvasRef.current?.focus();
  }, []);

  const gameTitle = useMemo(() => {
    if (status === "ready") return "Airplane Game";
    if (status === "loading") return "Preparing Airplane Game";
    if (status === "error") return "Game load failed";
    return "Click To Launch Airplane Game";
  }, [status]);

  return (
    <main className="app-shell">
      <section className="game-column" aria-labelledby="game-title">
        <div className="game-heading">
          <p className="eyebrow">Raylib C game in React</p>
          <h1 id="game-title">{gameTitle}</h1>
        </div>

        <GameCanvas
          canvasRef={canvasRef}
          frameRef={gameFrameRef}
          isFullscreen={isFullscreen}
          onToggleFullscreen={toggleFullscreen}
          status={status}
          statusText={statusText}
          onStart={startGame}
        />
      </section>

      <KeyBindingPanel
        disabled={!canSendInput}
        onKeyChange={sendGameKey}
      />
    </main>
  );
}

type GameCanvasProps = {
  canvasRef: RefObject<HTMLCanvasElement | null>;
  frameRef: RefObject<HTMLDivElement | null>;
  isFullscreen: boolean;
  onToggleFullscreen: () => void;
  status: GameStatus;
  statusText: string;
  onStart: () => void;
};

function GameCanvas({
  canvasRef,
  frameRef,
  isFullscreen,
  onToggleFullscreen,
  status,
  statusText,
  onStart,
}: GameCanvasProps) {
  return (
    <div
      ref={frameRef}
      className={`game-frame ${isFullscreen ? "is-fullscreen" : ""}`}
    >
      <button
        className="game-start-layer"
        data-hidden={status === "ready"}
        onClick={onStart}
        type="button"
      >
        <span>{statusText}</span>
      </button>
      <canvas
        ref={canvasRef}
        id="canvas"
        className="game-canvas"
        height={600}
        tabIndex={0}
        width={600}
      />
      <FullscreenToggle
        isFullscreen={isFullscreen}
        onToggle={onToggleFullscreen}
      />
    </div>
  );
}

type KeyBindingPanelProps = {
  disabled: boolean;
  onKeyChange: (code: number, isDown: boolean) => void;
};

function KeyBindingPanel({ disabled, onKeyChange }: KeyBindingPanelProps) {
  const releaseKey = useCallback(
    (code: number) => {
      onKeyChange(code, false);
    },
    [onKeyChange],
  );

  return (
    <aside className="key-panel" aria-label="Game keybindings">
      <div>
        <p className="eyebrow">Controls</p>
        <h2>Browser Keybinds</h2>
      </div>

      <div className="key-grid">
        {keyBindings.map((binding) => (
          <button
            className="key-button"
            disabled={disabled}
            key={binding.label}
            onBlur={() => releaseKey(binding.code)}
            onContextMenu={(event) => event.preventDefault()}
            onKeyDown={(event) => {
              if (event.key === " " || event.key === "Enter") {
                event.preventDefault();
              }
            }}
            onMouseDown={() => onKeyChange(binding.code, true)}
            onMouseLeave={() => releaseKey(binding.code)}
            onMouseUp={() => releaseKey(binding.code)}
            onTouchCancel={() => releaseKey(binding.code)}
            onTouchEnd={() => releaseKey(binding.code)}
            onTouchStart={(event) => {
              event.preventDefault();
              onKeyChange(binding.code, true);
            }}
            type="button"
          >
            <strong>{binding.label}</strong>
            <span>{binding.detail}</span>
          </button>
        ))}
      </div>
    </aside>
  );
}

type FullscreenToggleProps = {
  isFullscreen: boolean;
  onToggle: () => void;
};

function FullscreenToggle({ isFullscreen, onToggle }: FullscreenToggleProps) {
  return (
    <button className="fullscreen-toggle" onClick={onToggle} type="button">
      {isFullscreen ? "Window" : "Fullscreen"}
    </button>
  );
}

export default App;
