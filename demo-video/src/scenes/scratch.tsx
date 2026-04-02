import {makeScene2D, Rect, Txt} from '@motion-canvas/2d';
import {createRef, waitFor} from '@motion-canvas/core';

export default makeScene2D(function* (view) {
  const rect = createRef<Rect>();

  view.add(
    <Rect
      ref={rect}
      width={400}
      height={200}
      fill={'#1a1a2e'}
      radius={12}
    >
      <Txt
        text={'CodeForge Demo'}
        fill={'#D4D4D4'}
        fontSize={48}
        fontFamily={'monospace'}
      />
    </Rect>,
  );

  yield* rect().scale(0, 0).to(1, 0.6);
  yield* waitFor(1);
});
