/**
* Author: Will Roche
* Assignment: Lunar Lander
* Date due: 3/14/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/Entity.h"

// screen + game constants
constexpr int   SCREEN_WIDTH   = 1600;
constexpr int   SCREEN_HEIGHT  = 900;
constexpr int   FPS            = 60;
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
constexpr float WATER_GRAVITY  = 30.0f;
constexpr float THRUST         = 100.0f;  // acceleration in the ship's direction
constexpr float DRAG           = 0.2f;    // slows down by this factor when no key held
constexpr float FUEL_DRAIN     = 0.15f;   // fuel consumed per frame whenever moving

constexpr float ROTATION_SPEED = 200.0f;   // degrees per second
constexpr float ROTATION_MAX   = 90.0f;   // max tilt left or right (to get flat)

constexpr char  BG_COLOR[]  = "#0077B6";

constexpr float LANDER_W    = 30.0f;
constexpr float LANDER_H    = 90.0f;
constexpr float SHARK_W     = 120.0f;
constexpr float SHARK_H     = 60.0f;
constexpr int   NUM_SHARKS  = 3;
const float SHARK_SPEED[NUM_SHARKS] = { 150.0f, 250.0f, 200.0f };  // each shark has its own speed

constexpr float EXHAUST_W   = 40.0f;
constexpr float EXHAUST_H   = 30.0f;

constexpr float PLATFORM_W    = 140.0f;  // all platforms have same width
constexpr float HAZARD_H      = 60.0f;
constexpr float PLATFORM_H    = 35.0f;
constexpr float PLATFORM_Y    = 860.0f;  //all go at the bottom of the screen
constexpr int   NUM_PLATFORMS = 9;

// shark starting positions (staggered)
const float SHARK_X[NUM_SHARKS] = { 0.0f,   800.0f, 1200.0f };
const float SHARK_Y[NUM_SHARKS] = { 250.0f, 450.0f,  660.0f };

// enum for shark facing direction
enum SharkFacing { SHARK_LEFT, SHARK_RIGHT };

// delta time
AppStatus   gAppStatus       = RUNNING;
float       gPreviousTicks   = 0.0f;
float       gTimeAccumulator = 0.0f;

bool  gMissionFailed       = false;  // stays false until shrimp hits something
bool  gMissionAccomplished = false;  // stays false until shrimp lands on salvinia
float gFuel                = 100.0f; // depletes as thrust keys are held
bool  gThrustActive        = false;  // true only while KEY_UP is held and there's fuel remaining
float gAngle               = 0.0f;   // current ship angle

// lander and bubble exhaust
Entity *gLander  = nullptr;
Entity *gExhaust = nullptr;

// each shark has right and left entity, only right handles the physics
Entity     *gSharkRight[NUM_SHARKS]  = { nullptr, nullptr, nullptr };
Entity     *gSharkLeft[NUM_SHARKS]   = { nullptr, nullptr, nullptr };
SharkFacing gSharkFacing[NUM_SHARKS] = { SHARK_RIGHT, SHARK_RIGHT, SHARK_RIGHT };

// platform list, every other platform is safe
Entity *gPlatforms[NUM_PLATFORMS]     = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
bool    gPlatformIsWin[NUM_PLATFORMS] = { false, true, false, true, false, true, false, true, false };

// function declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();


//pulled from lec4

bool isColliding(const Vector2 *positionA, const Vector2 *scaleA, const Vector2 *positionB, const Vector2 *scaleB);
/**
 * @brief Checks for a square collision between 2 Rectangle objects.
 * 
 * @see 
 * 
 * @param positionA The position of the first object
 * @param scaleA The scale of the first object
 * @param positionB The position of the second object
 * @param scaleB The scale of the second object
 * @return true if a collision is detected,
 * @return false if a collision is not detected
 */
bool isColliding(const Vector2 *positionA,  const Vector2 *scaleA, 
                 const Vector2 *positionB, const Vector2 *scaleB)
{
    float xDistance = fabs(positionA->x - positionB->x) - ((scaleA->x + scaleB->x) / 2.0f);
    float yDistance = fabs(positionA->y - positionB->y) - ((scaleA->y + scaleB->y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}


void initialise(){
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Shrimpington Lander");
    SetTargetFPS(FPS);

    // starts near the top middle
    gLander = new Entity(
        { SCREEN_WIDTH / 2.0f, 80.0f },
        { LANDER_W, LANDER_H },
        "assets/game/RocketShrimp.png"
    );
    // start at 0, gravity will pull it down each frame
    gLander->setAcceleration({ 0.0f, WATER_GRAVITY });

    // sprite sheet 4 columns, 2 rows, 8 frames total (thank you to person from itch.io)
    gExhaust = new Entity(
        { SCREEN_WIDTH / 2.0f, 80.0f + LANDER_H / 2.0f + EXHAUST_H / 2.0f },
        { EXHAUST_W, EXHAUST_H },
        "assets/game/WaterSpritesheet.png",
        ATLAS, { 2, 4 },
        std::map<Direction, std::vector<int>>{ {DOWN, {0, 1, 2, 3, 4, 5, 6, 7}} }
    );

    // create all 3 sharks with the pre set positions
    for (int i = 0; i < NUM_SHARKS; i++){
        float startX = SHARK_X[i];

        // right-facing shark (also the physics entity)
        gSharkRight[i] = new Entity(
            { startX, SHARK_Y[i] },
            { SHARK_W, SHARK_H },
            "assets/game/SharkRight.png"
        );
        gSharkRight[i]->setVelocity({ SHARK_SPEED[i], 0.0f }); //SHARK ONLY TIME USING VELOCITY (never shrimp)

        // left-facing shark (copy the right one's position each frame)
        gSharkLeft[i] = new Entity(
            { startX, SHARK_Y[i] },
            { SHARK_W, SHARK_H },
            "assets/game/SharkLeft.png"
        );
    }

    //platforms cover the full bottom
    float spacing = (SCREEN_WIDTH - PLATFORM_W) / (float)(NUM_PLATFORMS - 1);

    for (int i = 0; i < NUM_PLATFORMS; i++){
        float x = PLATFORM_W / 2.0f + spacing * i;
        bool  isSafe = (i % 2 == 1); //checks if its one of the safe ones
        float h;
        const char *look;

        //pick if sea urchin or salvinia
        if (isSafe){
            h = PLATFORM_H;
            look = "assets/game/LandingPad.png";
        }
        else{
            h = HAZARD_H;
            look = "assets/game/Hazard.png";
        }

        gPlatforms[i] = new Entity({ x, PLATFORM_Y }, { PLATFORM_W, h }, look);
    }
}

void processInput(){
    if (WindowShouldClose()) gAppStatus = TERMINATED;

    // same end screen idea as project2, dont show anything after
    if (gMissionFailed || gMissionAccomplished) return;

    Vector2 accel = { 0.0f, WATER_GRAVITY };

    // left/right rotate the ship, no fuel cost (based on angle!!!!!!)
    if (IsKeyDown(KEY_LEFT))  gAngle -= ROTATION_SPEED * FIXED_TIMESTEP;
    if (IsKeyDown(KEY_RIGHT)) gAngle += ROTATION_SPEED * FIXED_TIMESTEP;

    //caps at flat angle like the real game
    if (gAngle < -ROTATION_MAX) gAngle = -ROTATION_MAX;
    if (gAngle >  ROTATION_MAX) gAngle =  ROTATION_MAX;

    gLander->setAngle(gAngle);

    // track previous state to detect a fresh key press for animation reset
    bool prevThrustActive = gThrustActive;
    gThrustActive = false;

    // pushes shrimp in the direction that it's is pointing
    if (IsKeyDown(KEY_UP) && gFuel > 0.0f){
        float angleRad = gAngle * DEG2RAD;
        accel.x +=  sinf(angleRad) * THRUST;
        accel.y += -cosf(angleRad) * THRUST;
        gFuel   -= FUEL_DRAIN;
        gThrustActive = true; //for animation reset
    }

    // restart bubble animation each time the key is freshly pressed
    //fixes issue with animation starting midway through after it already turned off
    if (gThrustActive && !prevThrustActive)
        gExhaust->resetAnimation(); //added to entity.h

    // drag opposes so the ship slows down
    accel.x += -(gLander->getVelocity().x) * DRAG; //only for x cause gravity does y

    if (gFuel < 0.0f) gFuel = 0.0f; //just in case

    gLander->setAcceleration(accel); //set to new post-input accel (NOT VELOCITY)
}

void update(){
    // if game over, freeze everything
    if (gMissionFailed || gMissionAccomplished) return;

    // delta time shenanigans
    float ticks     = (float)GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    // timestep accumulator, same as Activity 5
    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP){
        gTimeAccumulator = deltaTime;
        return;
    }
    //update everything w/ timestep
    while (deltaTime >= FIXED_TIMESTEP){
        gLander->update(FIXED_TIMESTEP);
        for (int i = 0; i < NUM_SHARKS; i++)
            gSharkRight[i]->update(FIXED_TIMESTEP);
        if (gThrustActive)
            gExhaust->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;
    }

    // needed with rotations
    // update hitbox to match the angle of the rotated shrimp
    float angle  = gAngle * DEG2RAD;
    float cosval = fabs(cosf(angle));
    float sinval = fabs(sinf(angle));
     gLander->setColliderDimensions({
        LANDER_W * cosval + LANDER_H * sinval,
        LANDER_W * sinval + LANDER_H * cosval
    });
    
    
    Vector2 landerPos = gLander->getPosition();
    // keep exhaust on the ship's bottom tip, rotated with the ship
    float exhaustDist     = LANDER_H / 2.0f + EXHAUST_H / 2.0f;
    //rotate with ship
    gExhaust->setPosition({
        landerPos.x + sinf(-angle) * exhaustDist,//NEEDS TO BE NEGATIVE SIN otherwise inverted
        landerPos.y + cosf(angle) * exhaustDist
    });
    gExhaust->setAngle(gAngle); //degrees not radians

    gTimeAccumulator = deltaTime;

    // sharks bounce off screen edges
    // + change direction too
    float halfW = SHARK_W / 2.0f;
    for (int i = 0; i < NUM_SHARKS; i++){
        // match left and right facing sharks
        gSharkLeft[i]->setPosition(gSharkRight[i]->getPosition());

        //math all based on right shark
        Vector2 pos = gSharkRight[i]->getPosition();
        Vector2 vel = gSharkRight[i]->getVelocity();

        //wall bouncing
        if (pos.x - halfW <= 0.0f){
            vel.x =  fabs(vel.x);  // must go right (away from left wall)
            gSharkFacing[i] = SHARK_RIGHT; //make it face right too for whichever gets rendered
            gSharkRight[i]->setVelocity(vel);
        }
        else if (pos.x + halfW >= SCREEN_WIDTH){
            vel.x = -fabs(vel.x);  // must go left (away from right wall)
            gSharkFacing[i] = SHARK_LEFT; //make it face left too for whichever gets rendered
            gSharkRight[i]->setVelocity(vel);
        }
    }

    // lander exits screen means mission fail (forbidden zone)
    // just 3 ors for up left and right (bottom is all covered with platforms)
    if (landerPos.x - LANDER_W / 2.0f < 0.0f         ||
        landerPos.x + LANDER_W / 2.0f > SCREEN_WIDTH  ||
        landerPos.y - LANDER_H / 2.0f < 0.0f){

        gMissionFailed = true;
    }

    // collision check for sharks (moving platform)
    for (int i = 0; i < NUM_SHARKS; i++){
        Vector2 lPos = gLander->getPosition();
        Vector2 lDim = gLander->getColliderDimensions();
        Vector2 sPos = gSharkRight[i]->getPosition();
        Vector2 sDim = gSharkRight[i]->getColliderDimensions();
        if (isColliding(&lPos, &lDim, &sPos, &sDim))
            gMissionFailed = true;
    }

    // collision check for bottom platforms
    for (int i = 0; i < NUM_PLATFORMS; i++){
        Vector2 lPos = gLander->getPosition();
        Vector2 lDim = gLander->getColliderDimensions();
        Vector2 pPos = gPlatforms[i]->getPosition();
        Vector2 pDim = gPlatforms[i]->getColliderDimensions();
        if (isColliding(&lPos, &lDim, &pPos, &pDim)){
            if (gPlatformIsWin[i] == true) //meaning win platform, have the bool vector above
                gMissionAccomplished = true;
            else
                gMissionFailed = true;
        }
    }
}

void render(){
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOR));

    // bubbles only show if currently in use
    if (gThrustActive)
        gExhaust->render();

    gLander->render();

    for (int i = 0; i < NUM_SHARKS; i++){
        if (gSharkFacing[i] == SHARK_LEFT) //sharks are rendered based on direction
            gSharkLeft[i]->render();
        else
            gSharkRight[i]->render();
    }

    // render all platforms
    for (int i = 0; i < NUM_PLATFORMS; i++)
        gPlatforms[i]->render();

    // fuel display top corner
    DrawText(TextFormat("Fuel: %d", (int)gFuel), 20, 20, 30, WHITE);

    // end game, similar to proj2 end game just with red and green colors
    if (gMissionFailed || gMissionAccomplished){
        const char *msg;
        Color color;
        if (gMissionFailed){
            msg   = "Mission Failed";
            color = RED;
        }
        else{
            msg   = "Mission Accomplished";
            color = GREEN;
        }
        int fontSize = 80;
        int textW    = MeasureText(msg, fontSize);
        DrawText(msg, (SCREEN_WIDTH - textW) / 2, SCREEN_HEIGHT / 2 - fontSize / 2, fontSize, color);
    }

    EndDrawing();
}

void shutdown(){
    // cleanup
    delete gLander;
    delete gExhaust;
    for (int i = 0; i < NUM_SHARKS; i++){
        delete gSharkRight[i];
        delete gSharkLeft[i];
    }
    for (int i = 0; i < NUM_PLATFORMS; i++)
        delete gPlatforms[i];
    CloseWindow();
}

int main(void){
    initialise();

    while (gAppStatus == RUNNING){
        processInput();
        update();
        render();
    }

    shutdown();
    return 0;
}
