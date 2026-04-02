// Motion Canvas 3 uses `Callback` as a global type in its .d.ts files,
// but does not declare it. This shim provides the definition.
type Callback = () => void;

// Vite virtual module suffix used by the Motion Canvas vite plugin.
declare module '*?scene' {
  import {FullSceneDescription} from '@motion-canvas/core/lib/scenes/Scene';
  const scene: FullSceneDescription;
  export default scene;
}
