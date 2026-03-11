#include "Entity.h"

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath) :
    mPosition {position}, mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
    mScale {scale}, mMovement {0.0f, 0.0f}, mColliderDimensions {scale},
    mTexture {LoadTexture(textureFilepath)}, mTextureType {SINGLE},
    mDirection {DOWN}, mAnimationAtlas {{}}, mAnimationIndices {},
    mFrameSpeed {DEFAULT_FRAME_SPEED}, mSpeed {DEFAULT_SPEED}, mAngle {0.0f} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath,
        TextureType textureType, Vector2 spriteSheetDimensions, std::map<Direction,
        std::vector<int>> animationAtlas) : mPosition {position},
        mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
        mMovement {0.0f, 0.0f}, mScale {scale}, mColliderDimensions {scale},
        mTexture {LoadTexture(textureFilepath)}, mTextureType {textureType},
        mSpriteSheetDimensions {spriteSheetDimensions},
        mAnimationAtlas {animationAtlas}, mDirection {DOWN},
        mAnimationIndices {animationAtlas.at(DOWN)},
        mFrameSpeed {DEFAULT_FRAME_SPEED}, mAngle {0.0f},
        mSpeed {DEFAULT_SPEED} { }

Entity::~Entity() { UnloadTexture(mTexture); };

/**
 * Updates the current frame index of an entity's animation based on the
 * elapsed time and frame speed.
 *
 * @param deltaTime represents the time elapsed since the last frame update.
 */
void Entity::animate(float deltaTime)
{
    mAnimationTime += deltaTime;
    float framesPerSecond = 1.0f / mFrameSpeed;

    if (mAnimationTime >= framesPerSecond)
    {
        mAnimationTime = 0.0f;

        mCurrentFrameIndex++;
        mCurrentFrameIndex %= mAnimationIndices.size();
    }
}

void Entity::update(float deltaTime)
{
    // apply acceleration to velocity, then velocity to position
    mVelocity.x += mAcceleration.x * deltaTime;
    mVelocity.y += mAcceleration.y * deltaTime;

    mPosition.x += mVelocity.x * deltaTime;
    mPosition.y += mVelocity.y * deltaTime;

    animate(deltaTime);
}

void Entity::render()
{
    Rectangle textureArea;

    switch (mTextureType)
    {
        case SINGLE:
            // Whole texture (UV coordinates)
            textureArea = {
                // top-left corner
                0.0f, 0.0f,

                // bottom-right corner (of texture)
                static_cast<float>(mTexture.width),
                static_cast<float>(mTexture.height)
            };
            break;
        case ATLAS:
            textureArea = getUVRectangle(
                &mTexture,
                mAnimationIndices[mCurrentFrameIndex],
                mSpriteSheetDimensions.x,
                mSpriteSheetDimensions.y
            );

        default: break;
    }

    // Destination rectangle – centred on gPosition
    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };

    // Origin inside the source texture (centre of the texture)
    Vector2 originOffset = {
        static_cast<float>(mScale.x) / 2.0f,
        static_cast<float>(mScale.y) / 2.0f
    };

    // Render the texture on screen
    DrawTexturePro(
        mTexture,
        textureArea, destinationArea, originOffset,
        mAngle, WHITE
    );
}
