#include <array>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

struct CRGB {
  int red, green, blue;
  CRGB(int r = 0, int g = 0, int b = 0) : red(r), green(g), blue(b) {}
  bool operator==(const CRGB& other) const {
    return red == other.red && green == other.green && blue == other.blue;
  }
  static const CRGB Black;
};
const CRGB CRGB::Black;
using Frame = std::array<CRGB, NUM_LEDS>;

// Every access, including clearing an already black LED, checks both bounds.
struct CheckedStrip {
  Frame values{};
  CRGB& operator[](int index) {
    if (index < 0 || index >= NUM_LEDS) {
      throw std::out_of_range("LED index " + std::to_string(index) +
                              " outside strip of " + std::to_string(NUM_LEDS));
    }
    return values[static_cast<std::size_t>(index)];
  }
} leds;

std::vector<Frame> frames;
std::vector<int> delays;
std::vector<int> positions;
struct FastLEDStub { void show() { frames.push_back(leds.values); } } FastLED;
struct SerialStub { void println(int position) { positions.push_back(position); } } Serial;
void delay(int milliseconds) { delays.push_back(milliseconds); }

// Includes the unmodified function bodies extracted from the production sketch.
#include "animations.inc"

void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void checkAnimation(void (*animate)(int), int steps, int speed) {
  leds.values.fill(CRGB::Black);
  frames.clear();
  delays.clear();
  positions.clear();
  animate(speed);
  require(static_cast<int>(frames.size()) == steps, "wrong number of displayed frames");
  require(static_cast<int>(delays.size()) == steps, "wrong number of delays");
  require(static_cast<int>(positions.size()) == steps, "wrong number of logged positions");
  for (int step = 0; step < steps; ++step) {
    require(delays[step] == speed, "animation speed changed");
    require(positions[step] == step, "animation iteration changed");
    for (int pixel = 0; pixel < NUM_LEDS; ++pixel) {
      CRGB expected = CRGB::Black;
      if (step > 1 && pixel == step - 1) expected = CRGB(0, 255, 0);
      if (pixel == step || pixel == NUM_LEDS - step - 1 ||
          (step > 0 && pixel == NUM_LEDS - step)) expected = CRGB(0, 150, 150);
      require(frames[step][pixel] == expected, "valid-pixel animation frame changed");
    }
  }
  for (const auto& pixel : leds.values) {
    require(pixel == CRGB::Black, "animation did not clear its LEDs");
  }
}

int main() {
  bool passed = true;
  for (int animation = 0; animation < 2; ++animation) {
    for (int speed : {0, 10, 50}) {
      try {
        if (animation == 0) checkAnimation(runTopToBottomLeds, NUM_LEDS / 2, speed);
        else checkAnimation(runBottomAndBackLeds, NUM_LEDS, speed);
      } catch (const std::exception& error) {
        std::cerr << (animation == 0 ? "top-to-bottom" : "bottom-and-back")
                  << ", NUM_LEDS=" << NUM_LEDS << ", speed=" << speed
                  << ": " << error.what() << '\n';
        passed = false;
      }
    }
  }
  if (!passed) return EXIT_FAILURE;
  std::cout << "Both animations pass bounds, frames, clearing and delay checks for "
            << NUM_LEDS << " LEDs at 3 speeds\n";
}
