# BC2 v0.44.0 Performance Profiler

This build is based on v0.43.0 Radio-Off.

Changes:
- Adds exact E-Paper timing logs:
  - render time
  - refresh time
  - total display time
  - full/partial path
  - partial refresh count
  - screen title
- Adds button press/release timestamps.
- Firmware version string updated to 0.44.0.
- No wallet, seed, PIN, signing, USB protocol, radio policy, or cryptographic behavior changed.

Expected log examples:
PERF button action=pressed timestamp=12345ms debounce=35ms
PERF display path=partial render=1ms refresh=760ms total=761ms partial_count=5 title=PIN EINGEBEN

Purpose:
Measure before changing the Waveshare display driver. The current partial path still
writes the complete 5000-byte framebuffer, so the next optimization will be based
on measured timings rather than assumptions.
