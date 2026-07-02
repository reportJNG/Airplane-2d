import { useCallback, useEffect, useRef, useState } from "react";
import type { RefObject } from "react";
import {
  ArrowLeft,
  ArrowRight,
  Crosshair,
  Maximize2,
  Minimize2,
  Pause,
  Play,
  RotateCcw,
} from "lucide-react";
import type { LucideIcon } from "lucide-react";
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

type KeyBinding = {
  action: string;
  code: number;
  Icon: LucideIcon;
  keys: string;
};

declare global {
  interface Window {
    Module?: AirplaneModule;
    airplaneGameScriptLoaded?: boolean;
  }
}

const keyBindings: KeyBinding[] = [
  { action: "Move left", code: 263, Icon: ArrowLeft, keys: "ArrowLeft / A" },
  { action: "Move right", code: 262, Icon: ArrowRight, keys: "ArrowRight / D" },
  { action: "Shoot", code: 32, Icon: Crosshair, keys: "Space" },
  { action: "Replay", code: 257, Icon: RotateCcw, keys: "Enter" },
  { action: "Pause", code: 256, Icon: Pause, keys: "Escape" },
];

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
  const [statusText, setStatusText] = useState("Play");
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
    setStatusText("Loading");

    window.Module = {
      canvas: canvasRef.current,
      locateFile: (path: string) => `/game/${path}`,
      onRuntimeInitialized: () => {
        setStatus("ready");
        setStatusText("Ready");
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
      setStatusText("Ready");
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
      setStatusText("Failed");
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

  return (
    <main className="app-shell">
      <GameCanvas
        canvasRef={canvasRef}
        frameRef={gameFrameRef}
        isFullscreen={isFullscreen}
        onStart={startGame}
        onToggleFullscreen={toggleFullscreen}
        status={status}
        statusText={statusText}
      />

      <KeyToolbar disabled={!canSendInput} onKeyChange={sendGameKey} />
    </main>
  );
}

type GameCanvasProps = {
  canvasRef: RefObject<HTMLCanvasElement | null>;
  frameRef: RefObject<HTMLDivElement | null>;
  isFullscreen: boolean;
  onStart: () => void;
  onToggleFullscreen: () => void;
  status: GameStatus;
  statusText: string;
};

function GameCanvas({
  canvasRef,
  frameRef,
  isFullscreen,
  onStart,
  onToggleFullscreen,
  status,
  statusText,
}: GameCanvasProps) {
  return (
    <section
      ref={frameRef}
      className={`game-frame ${isFullscreen ? "is-fullscreen" : ""}`}
      aria-label="Airplane game"
    >
      <button
        aria-label="Play game"
        className="game-start-layer"
        data-hidden={status === "ready"}
        disabled={status === "loading"}
        onClick={onStart}
        title="Play"
        type="button"
      >
        <span className="play-state">
          <Play aria-hidden="true" size={68} strokeWidth={1.8} />
          <span>{status === "idle" ? "Play" : statusText}</span>
        </span>
      </button>

      <canvas
        ref={canvasRef}
        id="canvas"
        className="game-canvas"
        height={600}
        tabIndex={0}
        width={600}
      />

      {status === "ready" && (
        <button
          aria-label={isFullscreen ? "Exit fullscreen" : "Fullscreen"}
          className="fullscreen-toggle"
          onClick={onToggleFullscreen}
          title={isFullscreen ? "Exit fullscreen" : "Fullscreen"}
          type="button"
        >
          {isFullscreen ? (
            <Minimize2 aria-hidden="true" size={24} />
          ) : (
            <Maximize2 aria-hidden="true" size={24} />
          )}
        </button>
      )}
    </section>
  );
}

type KeyToolbarProps = {
  disabled: boolean;
  onKeyChange: (code: number, isDown: boolean) => void;
};

function KeyToolbar({ disabled, onKeyChange }: KeyToolbarProps) {
  const releaseKey = useCallback(
    (code: number) => {
      onKeyChange(code, false);
    },
    [onKeyChange],
  );

  return (
    <nav className="key-toolbar" aria-label="Game controls">
      {keyBindings.map(({ action, code, Icon, keys }) => (
        <button
          aria-label={action}
          className="key-button"
          disabled={disabled}
          key={action}
          onBlur={() => releaseKey(code)}
          onContextMenu={(event) => event.preventDefault()}
          onKeyDown={(event) => {
            if (event.key === " " || event.key === "Enter") {
              event.preventDefault();
            }
          }}
          onMouseDown={() => onKeyChange(code, true)}
          onMouseLeave={() => releaseKey(code)}
          onMouseUp={() => releaseKey(code)}
          onTouchCancel={() => releaseKey(code)}
          onTouchEnd={() => releaseKey(code)}
          onTouchStart={(event) => {
            event.preventDefault();
            onKeyChange(code, true);
          }}
          title={`${action}: ${keys}`}
          type="button"
        >
          <Icon aria-hidden="true" size={26} strokeWidth={2.2} />
          <span className="key-copy">
            <strong>{action}</strong>
            <span>{keys}</span>
          </span>
        </button>
      ))}
    </nav>
  );
}

export default App;
