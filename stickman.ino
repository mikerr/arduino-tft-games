// Stickman
#include <TFT_eSPI.h> 
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();      

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x07FF
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

int color;

int button1 = 35;
int button2 = 0;

#define SCREENHEIGHT 120
#define SCREENWIDTH 230

typedef struct Vec2 {
  float x;
  float y;
} vec2;

Vec2 vecadd( vec2 p1, Vec2 p2) {
  Vec2 result;
  result.x = p1.x + p2.x;
  result.y = p1.y + p2.y;
  return result;
}

int speed;
int size  = 20;
float dustx,count;

Vec2 center,neck,hip;

struct bone { Vec2 joint1; Vec2 joint2; };
std::vector<bone> bones;

float deg2rad( float deg){ return ( (deg / 180.0f) * 3.142); }

void add_leg(int side) {
float angle;
Vec2 knee,ankle;

    angle = sin(count + side) * 30;
    angle = deg2rad(angle +170);

    knee = {sin (angle) * size, -cos (angle) * size};
    knee = vecadd(knee,hip);
  
    bones.push_back( {hip,knee} );

    angle = sin (count + side - 0.873f) * 45;
    angle = deg2rad(angle + 225);

    ankle = {sin (angle) * size  , -cos (angle) * size};
    ankle = vecadd(ankle,knee);
    
    bones.push_back ( {knee,ankle} );
}

void add_arm(int side) {
float angle;
Vec2 shoulder,elbow,wrist;
int armsize  = size  * 0.75;
    shoulder.y = neck.y + size/2;

    bones.push_back ( {shoulder,neck} );

    angle = deg2rad(sin(count + side) * 60);

    elbow = {sin (angle) * armsize, cos (angle) * armsize};
    elbow = vecadd(elbow,shoulder);
    bones.push_back ( {shoulder,elbow} );

    angle = sin(count + side) * 60;
    angle = deg2rad(angle + 50);

    wrist = {sin (angle) * armsize, cos (angle) * armsize};
    wrist = vecadd(wrist,elbow);
    bones.push_back ( {elbow,wrist} );
}

void add_torso(void) {

    neck = {0,-size*1.5};
    neck = vecadd( neck,hip);
    bones.push_back ( {hip,neck} );
}

void draw_head(void) {
    Vec2 head;
    head = vecadd(center, neck);
    head.y -= size  /2;
    tft.drawCircle(head.x, head.y, size  / 2, WHITE);
}

void drawground ( void) {
    for(int dust=0; dust < SCREENWIDTH + 200; dust += 40) 
      tft.drawPixel (dustx + dust,SCREENHEIGHT,GREEN); 
}

void read_buttons( void ) {
  if (digitalRead(button1) == LOW) size++;
  if (digitalRead(button2) == LOW) size--;
}
void setup()
{
  // Setup the LCD
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(BLACK);

  pinMode(button1, INPUT);
  speed = 400;
  center = { SCREENWIDTH /2, SCREENHEIGHT /2 };
}

void loop()
{
  float left  = 0;
  float right = 3.142;
  
    tft.fillScreen(TFT_BLACK);
    read_buttons();
    // draw moving dust on ground
    count += speed / 2000.0f;

    dustx -= speed / 50.0f;
    if (dustx < -160) dustx = 0;
    
    drawground();

    // draw stickman
    color = WHITE;
    bones.clear();

    add_leg(left);
    add_leg(right);

    add_torso();

    add_arm(left);
    add_arm(right);

    for (auto bone:bones) {
      Vec2 line = vecadd(bone.joint1, center);
      Vec2 line2 = vecadd(bone.joint2, center);
      tft.drawLine(line.x, line.y, line2.x, line2.y, WHITE);
    }
    draw_head();

delay(50);
}
