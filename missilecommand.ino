// missile command game

#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x07FF
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

int colors[] = {BLACK,BLUE,RED,MAGENTA,GREEN,CYAN,YELLOW,WHITE};
int color;

int button1 = 35;
int button2 = 0;

#define SCREENHEIGHT 120
#define SCREENWIDTH 230

typedef struct Vec2 {
  float x;
  float y;
} vec2;

typedef struct missile {
    Vec2 start;
    Vec2 pos;
    Vec2 velocity;
    bool trail;
} missile;

std::vector<missile> missiles;

typedef struct shape {
    std::vector<Vec2> points;
    bool hit;
} shape;

shape cities[3];
    
bool fire;
int ground,score;
Vec2 firepos;

void draw_shape(shape shape){
    Vec2 lastpos = shape.points[0];
    for (auto &p: shape.points) {
        tft.drawLine(lastpos.x, lastpos.y , p.x, p.y, color);
        lastpos = p;
    }
}
void draw_cross(Vec2 pos) {
    int s = 5;
    color = RED;
    tft.drawLine(pos.x - s, pos.y - s, pos.x + s, pos.y + s, color);    
    tft.drawLine(pos.x + s, pos.y - s, pos.x - s, pos.y + s, color);
}

void draw_laser(Vec2 pos) {
    color = colors[rand() %7];
    tft.drawLine(0,SCREENHEIGHT,pos.x,pos.y,color);
    tft.drawLine(SCREENWIDTH, SCREENHEIGHT, pos.x,pos.y,color);
}

bool hit ( Vec2 p1,Vec2 p2) {
  int distance = abs(p2.x-p1.x) + abs(p1.y-p2.y);
  if (distance < 10) return true;
  return false;
}
bool point_in_shape ( Vec2 point, std::vector<Vec2> shape ){
// check a given point is enclosed by a polygon
// http://erich.realtimerendering.com/ptinpoly/ for an explanation !

  int i, j, nvert = shape.size();
  bool c = false;

  for(i = 0, j = nvert - 1; i < nvert; j = i++) { 
    if( ( (shape[i].y >= point.y ) != (shape[j].y >= point.y) ) &&
        (point.x <= (shape[j].x - shape[i].x) * (point.y - shape[i].y) / (shape[j].y - shape[i].y) + shape[i].x)
      ) 
      c = !c;
  }
  return c;
}
bool offscreen (Vec2 pos) {
    if (pos.x > SCREENWIDTH || pos.x < 0 ) return(true);
    if (pos.y > SCREENHEIGHT || pos.y < 0 ) return(true);
    return (false);
}

float rnd (int max) {
    float random = rand() % (max * 10);
    random = random / 10.0f;
    return (random);
}
void setup()
{
  // Setup the LCD
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(BLACK);

  pinMode(button1, INPUT);
  
   for (int i=0; i < 10; i++ ) {
        missile m;
        missiles.push_back(m);
        }
    firepos = {100,75};
    ground = SCREENHEIGHT;
    int city = ground - 10;
    cities[0].points = { {0,ground},{40,ground},{80,city},{120,city},{160,ground},{200,ground}};
    cities[1].points = cities[2].points = cities[0].points;

    for (int c = 0; c < 3; c++)
      for ( auto &p : cities[c].points) {
      static int x;
      p.x = x;
      x = x + SCREENWIDTH / 16;
    }
  
}

void loop()
{
  tft.fillScreen(TFT_BLACK);

  static int heat;
  int citiesleft = 3;

  //draw cities
  for (auto &city : cities) {
        if (city.hit) {
          color = RED;
          citiesleft--; 
        }
        else color = GREEN;
        draw_shape(city);
  }
  // draw missiles
  for (auto &m : missiles) {
        m.pos.x += m.velocity.x;
        m.pos.y += m.velocity.y;  
        color = CYAN;
        if (m.trail) tft.drawLine(m.start.x, m.start.y, m.pos.x, m.pos.y, color);
        else tft.drawPixel(m.pos.x, m.pos.y, color);

        // spawn new missile when they go off screen
        if (offscreen(m.pos) || m.velocity.y == 0 ) {
            m.start = {rand () % SCREENWIDTH, 0};
            m.pos = m.start;
            m.velocity = {1.0f - rnd(2),1};
            if (rnd(100) < 90) m.trail = true;
            else m.trail = false;
        }
        // check for missile & city collision
        for (auto &city : cities) {
            if (point_in_shape(m.pos,city.points)) {
                 m.velocity.y = 0;
                 tft.drawString("City destroyed !",0,SCREENHEIGHT / 2);
                 color = RED;
                 draw_shape(city);
                 city.hit = true;
                 // level city to the ground
                 for (auto &p : city.points) 
                     p.y = ground;
            }
        }
    }

    draw_cross(firepos);

    fire = 1;
    if (digitalRead(button1) == LOW) {
      fire = 0; 
      firepos.x += 5;
    }
    if (digitalRead(button2) == LOW) {
      fire = 0;
      firepos.x -= 5;
    }

    // laser overheats if fired for 2 seconds
    if (fire && heat < 120) {
         draw_laser(firepos);
         heat+= 5;
         for (auto &m : missiles) {
            if (hit (m.pos , firepos))
                  m.velocity.y = 0;
            }
     }

    //draw laser temperature line
    color = RED;
    //tft.drawLine(SCREENWIDTH,SCREENHEIGHT,SCREENWIDTH,SCREENHEIGHT - heat,color);
    if (!fire && heat > 0) heat = heat - 2;

    if (citiesleft) score++;
    else tft.drawString("Cities destroyed - all is lost !",0,SCREENHEIGHT /2);
    //screen.text(std::to_string(score / 10),minimal_font,Vec2(0,screen.bounds.h - 10));
    
delay(50);
}
