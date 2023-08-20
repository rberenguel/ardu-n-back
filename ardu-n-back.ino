#include <Arduboy2.h>
#include <Tinyfont.h>
Arduboy2 arduboy;

Tinyfont tinyfont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

// A bit messy, but it's an implementation of the dual N-Back task (using positions and letters)
// for the Arduboy. Fixed N = 2 so far.

#define N 2
#define W 42
#define H 18
#define GW 16
#define GH 6
int currentLet;
int currentPos;
int previousLet[N + 1];
int previousPos[N + 1];
int total = 0;
int rightPos = 0;
int rightLet = 0;

boolean isCorrect;
byte positions[9][2] = {
  { GW, GH }, // TOP_LEFT
  { 3 + GW + W, GH }, // TOP_CENTER
  { 4 + GW + 2 * W, GH }, // TOP_RIGHT
  { GW, GH + H }, // MID_LEFT
  { 3 + GW + W, GH + H }, // MID_CENTER
  { 4 + GW + 2 * W, GH + H }, // MID_RIGHT
  { GW, GH + 2 * H }, // BOTTOM_LEFT
  { 3 + GW + W, GH + 2 * H }, // BOTTOM_CENTER
  { 4 + GW + 2 * W, GH + 2 * H }, // BOTTOM_RIGHT
};

char letters[9] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I' };

const int TOP_LEFT = 0;
const int TOP_CENTER = 1;
const int TOP_RIGHT = 2;
const int MIDDLE_LEFT = 3;
const int MIDDLE_CENTER = 4;
const int MIDDLE_RIGHT = 5;
const int BOTTOM_LEFT = 6;
const int BOTTOM_CENTER = 7;
const int BOTTOM_RIGHT = 8;

enum Answer {
  NOTHING,
  OTHER,
  SAME_POS,
  SAME_LET,
  BOTH_SAME,
  ALL_DIF
};

Answer answer;

enum State {
  INSTRUCTIONS,
  WAIT,
  ASK
};

State state;

void instructions() {
  arduboy.print("\n   UP:    BOTH_SAME");
  arduboy.print("\n   DOWN:  ALL_DIFF");
  arduboy.print("\n   RIGHT: SAME_LET");
  arduboy.print("\n   LEFT:  SAME_POS");
  arduboy.display();
  state = INSTRUCTIONS;
}

void setup() {
  arduboy.boot();
  arduboy.initRandomSeed();
  arduboy.clear();

  arduboy.setCursor(0, 0);

  arduboy.print("Welcome to the\ndual n-back task!\n");
  instructions();
  for (int i = 0; i < N + 1; i++) {
    previousLet[i] = -1;
    previousPos[i] = -1;
  }
  answer = NOTHING;
}

void debug() {
  tinyfont.setCursor(0, H * 3 - 4);
  for (int i = 0; i < N + 1; i++) {
    tinyfont.print(previousLet[i]);
    tinyfont.print(" ");
    tinyfont.print(previousPos[i]);
    tinyfont.print(" ");
  }
  tinyfont.print("\n");
  miniStats();
}

void miniStats() {
  float perPos = (rightPos * 100.0 / (total - N));
  float perLet = (rightLet * 100.0 / (total - N));
  tinyfont.print("- ");
  tinyfont.print(perPos);
  tinyfont.print(" A ");
  tinyfont.print(perLet);
  tinyfont.print("%");
  tinyfont.print("  ");
  tinyfont.print(total - N);
}

void largeStats() {
  float perPos = (rightPos * 100.0 / (total - N));
  float perLet = (rightLet * 100.0 / (total - N));
  arduboy.print("\x1A ");
  arduboy.print(perPos);
  arduboy.print(" A ");
  arduboy.print(perLet);
  arduboy.print("%");
  arduboy.print("  ");
  arduboy.print(total - N);
}

void status() {
  //debug();
  //return;
  arduboy.setCursor(0, H * 3 + GH / 2);
  if (total <= N) {
    if (total == N) {
      arduboy.print("Now you can answer");
    } else {
      arduboy.print("");
    }
  } else {
    largeStats();
  }
}

void createQuestion() {
  currentLet = random(9);
  currentPos = random(9);

  // Update the previous letters and positions arrays. Oldest at the "bottom"
  for (int i = 0; i < N + 1; i++) {
    previousLet[i] = previousLet[i + 1];
    previousPos[i] = previousPos[i + 1];
  }
  previousLet[N] = currentLet;
  previousPos[N] = currentPos;

  asking();
  total++;
}

void grid(){
  arduboy.drawFastHLine(0, 1, Arduboy2::width());
  arduboy.drawFastHLine(0, H, Arduboy2::width());
  arduboy.drawFastHLine(0, 2*H, Arduboy2::width());
  arduboy.drawFastHLine(0, 3*H, Arduboy2::width());
  arduboy.drawFastVLine(W, 1, 3*H);
  arduboy.drawFastVLine(2*W, 1, 3*H);
}

void asking() {
  arduboy.clear();
  grid();
  int x = positions[currentPos][0];
  int y = positions[currentPos][1];

  arduboy.setCursor(x, y);

  arduboy.print(letters[currentLet]);
  status();
  arduboy.display();
  state = WAIT;
}

void loop() {

  // ASK -> WAIT  Ask is an internal state
  // WAIT -> ASK when answered
  // WAIT -> INSTRUCTIONS when A or B
  // INSTRUCTIONS -> WAIT when A or B
  
  arduboy.pollButtons();
  if (state == ASK) {
    createQuestion();
    return;
  }

  // Get the user's answer.
  if (arduboy.justPressed(A_BUTTON)) {
    answer = OTHER;
  }
  if (arduboy.justPressed(B_BUTTON)) {
    answer = OTHER;
  }
  if (arduboy.justPressed(UP_BUTTON)) {
    answer = BOTH_SAME;
  }
  if (arduboy.justPressed(DOWN_BUTTON)) {
    answer = ALL_DIF;
  }
  if (arduboy.justPressed(RIGHT_BUTTON)) {
    answer = SAME_LET;
  }
  if (arduboy.justPressed(LEFT_BUTTON)) {
    answer = SAME_POS;
  }

  if(answer == NOTHING && state == INSTRUCTIONS){
    return;
  }

  if(answer == NOTHING && state == WAIT){
    return;
  }

  if(answer == OTHER){
    // Pressed A or B
    answer = NOTHING;
    if(total <= N){
      // Regardless, move to the next question
      state = ASK;
      return;
    }
    if(state == INSTRUCTIONS){
      // We are showing the instructions, switch to questioning
      asking();
      return;
    } else {
      // Otherwise, show the instructions
      arduboy.clear();
      instructions();
      return;
    }
  }

  bool sameLet = currentLet == previousLet[0];
  bool samePos = currentPos == previousPos[0];
  if (answer == BOTH_SAME) {
    if (sameLet) {
      rightLet += 1;
    }
    if (samePos) {
      rightPos += 1;
    }
  }
  if (answer == ALL_DIF) {
    if (!sameLet) {
      rightLet += 1;
    }
    if (!samePos) {
      rightPos += 1;
    }
  }
  if (answer == SAME_POS) {
    if (!sameLet) {
      rightLet += 1;
    }
    if (samePos) {
      rightPos += 1;
    }
  }
  if (answer == SAME_LET) {
    if (sameLet) {
      rightLet += 1;
    }
    if (!samePos) {
      rightPos += 1;
    }
  }
  answer = NOTHING;
  state = ASK;
}